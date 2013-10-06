#!/usr/bin/perl
#
# getnews.pl -- get news from Asahi Shimbun twitter (@asahi)
#
# * This file should be saved in UTF-8. *
#

use Encode;

$TW_CMD = '/usr/local/bin/tw19';
$NEWS_ID = '@asahi';
$NEWS_FILE = './mb/news.sjis';

@news = `$TW_CMD $NEWS_ID`;

open(NEWS, "> $NEWS_FILE") || die;

foreach $news (@news) {
    # I do not need YouTube news
    next if $news =~ m/\@YouTube/;
    # Delete date
    $news =~ s/\[\d\d\/\d\d\ .*\]\s+//;
    # Delete time, but use it later
    $news =~ s/\((\d\d)\:(\d\d)\:\d\d\)\s+//;
    $time = "(" . $1 . ":" . $2 .")";
    # Delete '@asahi'
    $news =~ s/\@asahi\s+\:\s+//;
    # Delete URL
    $news =~ s/\:?\s*(http\:\/\/.*)\s*//;
    # Add header
    $news = "【朝日新聞】" . $news . $time;
    # convert to Shift-JIS
    print NEWS encode('cp932', decode_utf8($news)), "\n";
}

close(NEWS);
