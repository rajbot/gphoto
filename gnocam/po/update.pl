#!/usr/bin/perl -w

#  GNOME PO Update Utility.
#  (C) 2000 The Free Software Foundation
#
#  Author(s): Kenneth Christiansen
#
#  GNOME PO Update Utility requires the XML to POT Generator, xml2pot.pl
#  Please distribute it along with this scrips, aswell as desk.po and
#  README.tools.
#
#  Also remember to change $PACKAGE to reflect the package the script is
#  used within.


$VERSION = "1.3";
$LANG    = $ARGV[0];
$PACKAGE  = "GnoCam";
$| = 1;


if (! $LANG){
    print "update.pl:  missing file arguments\n";
    print "Try `update.pl --help' for more information.\n";
    exit;
}

if ($LANG=~/^-(.)*/){

    if ("$LANG" eq "--version" || "$LANG" eq "-V"){
        &Version;
    }
    elsif ($LANG eq "--help" || "$LANG" eq "-H"){
	&Help;
    }
    elsif ($LANG eq "--dist" || "$LANG" eq "-D"){
        &Merging;
#       &Status;
    }
    elsif ($LANG eq "--pot" || "$LANG" eq "-P"){
   	&GeneratePot;
        exit;
    }
    elsif ($LANG eq "--maintain" || "$LANG" eq "-M"){
        &Maintain;
    }
    else {
        &InvalidOption;
    }

} else {
   if(-s "$LANG.po"){
	&GeneratePot;
	&Merging;
	&Status;
   }  
   else {
	&NotExisting;       
   }
}

sub Version{
    print "GNOME PO Updater $VERSION\n";
    print "Written by Kenneth Christiansen <kenneth\@gnome.org>, 2000.\n\n";
    print "Copyright (C) 2000 Free Software Foundation, Inc.\n";
    print "This is free software; see the source for copying conditions.  There is NO\n";
    print "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
    exit;
}

sub Help{
    print "Usage: ./update.pl [OPTIONS] ...LANGCODE\n";
    print "Updates pot files and merge them with the translations.\n\n";
    print "  -V, --version                shows the version\n";
    print "  -H, --help                   shows this help page\n";
    print "  -P, --pot                    only generates the potfile\n";
#   print "  -S, --status                 shows the status of the po file\n";
    print "  -M, --maintain               search for missing files in POTFILES.in\n";
    print "\nExamples of use:\n";
    print "update.sh --pot    just creates a new pot file from the source\n";
    print "update.sh da       created new pot file and updated the da.po file\n\n";
    print "Report bugs to <kenneth\@gnome.org>.\n";
    exit;
}

sub Maintain{
    $a="find ../ -print | egrep '.*\\.(c|y|cc|c++|h|gob)' ";

    open(BUF2, "POTFILES.in") || die "update.pl:  there's not POTFILES.in!!!\n";
    
    print "Searching for missing _(\" \") entries...\n";
    
    open(BUF1, "$a|");

    @buf2 = <BUF2>;
    @buf1 = <BUF1>;

    if (-s "POTFILES.ignore"){
        open FILE, "POTFILES.ignore";
        while (<FILE>) {
            if ($_=~/^[^#]/o){
                push @bup, $_;
            }
        }
        print "POTFILES.ignore found! Ignoring files...\n";
        @buf2 = (@bup, @buf2);
    }

    foreach my $file (@buf1){
        open FILE, "<$file";
        while (<FILE>) {
            if ($_=~/_\(\"/o){
                $file = unpack("x3 A*",$file) . "\n";
                push @buff1, $file;
                last;
            }
        }
    }

    @bufff1 = sort (@buff1);
    @bufff2 = sort (@buf2);

    my %in2;
    foreach (@bufff2) {
       $in2{$_} = 1;
    }

    foreach (@bufff1){
       if (!exists($in2{$_})){
           push @result, $_ }
       }

    if(@result){
        open OUT, ">POTFILES.in.missing";
        print OUT @result;
        print "\nHere are the results:\n\n", @result, "\n";
        print "File POTFILES.in.missing is being placed in directory...\n";
        print "Please add the files that should be ignored in POTFILES.ignore\n";
    }
    else{
        print "\nWell, it's all perfect! Congratulation!\n";
    }         
}

sub InvalidOption{
    print "update.pl: invalid option -- $LANG\n";
    print "Try `update.pl --help' for more information.\n";
}
 

sub GeneratePot{

    print "Building the $PACKAGE.pot...\n\n";

    $c="xgettext --default-domain\=$PACKAGE --directory\=\.\."
      ." --add-comments --keyword\=\_ --keyword\=N\_"
      ." --files-from\=\.\/POTFILES\.in ";  
    $c1="test \! -f $PACKAGE\.po \|\| \( rm -f \.\/$PACKAGE\-source.pot "
       ."&& mv $PACKAGE\.po \.\/$PACKAGE\-source.pot \)";

    system($c);
    system($c1);
}

sub Merging{

    if ($ARGV[1]){
        $LANG   = $ARGV[1];
    } else {
	$LANG   = $ARGV[0];
    }

    if ($ARGV[0] ne "--dist" && $ARGV[0] ne "-D") {
        print "\n\nMerging $LANG.po with $PACKAGE.pot, creating updated $LANG.po...\n\n";
    }

    $d="cp $LANG.po $LANG.po.old && msgmerge $LANG.po.old $PACKAGE.pot -o $LANG.po";

    if ($ARGV[0] ne "--dist" && $ARGV[0] ne "-D") {
        print "Working, please wait";
    }
    system($d);
    
    if ($ARGV[0] ne "--dist" && $ARGV[0] ne "-D") {
        print "\n\n";
    }

    unlink "messages";
    unlink "$LANG.po.old";
}

sub NotExisting{
    print "update.pl:  sorry $LANG.po does not exist!\n";
    print "Try `update.pl --help' for more information.\n";    
    exit;
}

sub Status{
    $f="msgfmt --statistics $LANG.po";
    
    system($f);
    print "\n";   
}
