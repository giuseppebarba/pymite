#
# PyMite - A flyweight Python interpreter for 8-bit and larger microcontrollers.
# Copyright 2002 Dean Hall.  All rights reserved.
# PyMite is offered through one of two licenses: commercial or open-source.
# See the LICENSE file at the root of this package for licensing details.
#

import sys;
import list;
import dict;
import string;
import ipm;

def nulladd(x):
	return(x+1);

def nulllib(y):
	print ;
	return(y - 355);

j=0;
### dir(__builtins__);
print sys.time(); # will return zero... not implemented yet X-D

a="hello world!";
print "=== check: string";
"hello world!";
a;

print "=== check: print string";
print "hello world!";
print a;
print len("hello world!");
print len(a);
##print length(a[2:7]);  ## slicees are not implemented yet 
##print a[2:7];  ## slicees are not implemented yet 

print "=== check: print number";
a=355; b=113;
print 355/113, 0, a, b, a/b*a, -b/a;
print "=== check: print number in format";
print "%08d / %8d =%-8d" % (355,113,355/113);
print "0x%02x / 0x%04X =%04x" % (355,113,355/113);

print "=== check: print floating";
print 355.0/113.0;
print 355.0/3*3.0;
print 1/3.0, 1/3.0*3.0;
print "=== check: print format";
print "%d / %d =%32.30f" % (355,113,355.0/113.0);

print "\r\n\r\n=== check: list";
a=[4,5,6,7,8];
print [];
print a;
print len(a);
##print a[2:4];  ## slicees are not implemented yet 
##print a[:];  ## slicees are not implemented yet 
print a[0],a[-1];
for x in a:
	print x,x*x;

print "=== check: tupple";
a=(9,8,7,6,5,4,3,2,1);
print ();
print a;
print len(a);
print a[0],a[-1];
##print a[2:5];  ## slicees are not implemented yet 
##print a[:];  ## slicees are not implemented yet 
for x in a:
	print x,x*x;
print "=== check: dict";
a={"four":4,"five":5,"six":6};
print a;
print a["four"];
print len(a);
print "six" in a;
print "one" in a;
print dict.keys(a);
print "=== check: for loop 10 print and if statement";
for i in range(0,11):
	print i;
	if(i==5):
		print "if-then %-22.20f" % (1.0/i);
	if(i<3):
		print "if-(then)-else %32.32f" % (1.0*i);
	else:
		print "if-then-(else) %-.32f" % (1.0/i);
print "=== check: for loop 100 func-call-loop";
for i in range(0,100):
	j= nulladd(i)+j;
j= nulllib(j);
print j

print "=== check: sys module-*";
print sys.heap();
sys.putb(0x31);sys.putb(0x31);sys.putb(0x39);sys.putb(0x32); ## will display 1192
print "\r\n";
print sys.getb(); ## -1=255 returned
print sys.time(); # will return zero... not implemented yet X-D
#sys.wait(150); ## waiting 150ms ... looping :-<
#print sys.time();

print "=== check: string module-atoi";
print string.atoi("-123");
print string.atoi("   -123",8);
print string.atoi("-123",16);
##print string.atoi("-123   "); ## error trailing junk chars.

print "=== check: string module-find";
print string.find("-123",'-');
print string.find("-123",'3');
print string.find("-123",'a');
print string.find("-123",'');

print "=== check: string module-count";
print string.count("abc-123+xyzab",'a');
print string.count("abc-123+xyzab",'b');
print string.count("abc-123+xyzab",' ');
print string.count("abc-123+xyzab",'123');
print string.count("abc-123+xyzab",'+123');

print "=== check: ipm module-ipm()";
ipm.ipm();


sys.exit();

# :mode=c:
