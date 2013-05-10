#!/bin/sh


g++ -fPIC -shared -o mod_soap.so mod_soap.c test.o -I/usr/include/apr-1.0 -I/usr/include/openssl -I/usr/include/xmltok -pthread     -I/usr/include/apache2  -I/usr/include/apr-1.0   -I/usr/include/apr-1.0 -I/usr/include/httpd -std=c++0x


sudo cp mod_soap.so /usr/lib/apache2/modules/mod_soap.so

sudo chmod 644 /usr/lib/apache2/modules/mod_soap.so

