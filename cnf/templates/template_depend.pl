#! /usr/bin/perl

while (<>)
{
    if (m/[<]TMPL_include\s+\"([^\"]+)\"[>]/i)
    {
	print "$1 ";
    }
}
