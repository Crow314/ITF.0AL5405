#!/bin/sh -
# usage: [VERBOSE=1] fstest.sh fsprog

print ()
{
    echo "$@" 1>&2
}

message ()
{
    if [ "$VERBOSE" ]
    then
    print "$@"
    fi
}

try ()
{
    if [ "$VERBOSE" ]
    then
    sh -c "$1"
    else
    sh -c "$1" 2>/dev/null
    fi
}

ERROR_OCCURRED=

error ()
{
    if [ "$STOP_FIRST" ]
    then
	exit 1
    fi
    ERROR_OCCURRED=1
}

should_success ()
{
    message "Executing [$1] (should succeed)"
    try "$1" || {
    print "Command [$1] failed"; error
    }
    message "ok"
}

should_fail ()
{
    message "Executing [$1] (should fail)"
    try "$1" && {
    echo "Command [$1] (which should fail) succeeded" 1>&2 ; error
    }
    message "ok"
}

DIR="tmpdir$$"

message "Making directory $DIR"
should_success "mkdir $DIR"

message "Starting fs $1 on $DIR"
should_success "$1 $DIR"
cd $DIR

# file1を作る
should_success "echo HELLO >file1"

# file1をfile2にコピーする
should_success "cp file1 file2"

# file1とfile2の内容が同じか確認
should_success "diff file1 file2"

# file1の名前をfile1_1に変更する
should_success "mv file1 file1_1"

# file1というファイル(もう無いはず)を削除する
should_fail "rm file1"

# file1_1を削除する
should_success "rm file1_1"

# file1_1(もう無いはず)を削除する
should_fail "rm file1_1"

#---------------------

# もう一度 file1を作る
should_success "echo HELLO >file1"

# file1の名前をfile2に変更する
should_success "mv file1 file2"

# file1というファイル(もう無いはず)を削除する
should_fail "rm file1"

# file2をfile3にコピーする
should_success "cp file2 file3"

# file3の名前をfile2に変更する
should_success "mv file3 file2"

# file3というファイル(もう無いはず)を削除する
should_fail "rm file3"

#---------------------

# file2のサイズを大きくする(22ブロック)
#should_success "dd if=/dev/random seek=20 count=1 of=file2"
should_success "dd if=/dev/zero seek=21 count=1 of=file2"

# file2をfile2_2にコピーする
should_success "cp file2 file2_2"

# file2のサイズをさらに大きくする(31ブロック)
should_success "dd if=/dev/zero seek=30 count=1 of=file2"

# file2のサイズを22ブロックに戻す
should_success "dd if=/dev/zero seek=21 count=1 of=file2"

# file2とfile2_2の内容が同じか確認
should_success "cmp file2 file2_2"

# file2のサイズをゼロにする
should_success "dd count=0 of=file2"

# file2が空ファイルか確認
should_success "diff /dev/null file2"

# file2のサイズを再び大きくする
should_success "dd if=/dev/zero seek=20 count=1 of=file2"

# サイズをゼロにする以前のデータが残ってしまっていないか確認
should_fail "diff file2 file2_2 >/dev/null"

# file2の内容が，21ブロックすべてヌル文字であるか確認
should_success "dd if=/dev/zero count=21 | diff - file2"

message "Killing `basename $1`"
killall `basename "$1"`
sleep 1
cd ..
rm -r "$DIR"

if [ "$ERROR_OCCURRED" ]
then
    exit 1
else
    print "all ok!"
    exit 0
fi
