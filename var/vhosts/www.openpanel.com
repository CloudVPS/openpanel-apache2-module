<VirtualHost 10.0.2.143:80>
   ServerAdmin        webmaster@openpanel.com
   DocumentRoot       ./home/testuser/html/www.openpanel.com
   ServerName         www.openpanel.com
   ErrorLog           ./home/testuser/log/www.openpanel.com.err
   CustomLog          ./home/testuser/log/www.openpanel.com.log combined
   AllowOverride      All
   <Directory ./home/testuser/html/www.openpanel.com>
       AddType text/plain .php .php3 .phtml
       AddType text/plain .phps
       AddHandler cgi-script .cgi
       AddType text/plain .pl
   </Directory>
</VirtualHost>
