<VirtualHost 10.0.2.143:80>
   ServerAdmin        webmaster@openpanel.com
   DocumentRoot       ./home/testuser/html/admin.openpanel.com
   ServerName         admin.openpanel.com
   ErrorLog           ./home/testuser/log/admin.openpanel.com.err
   CustomLog          ./home/testuser/log/admin.openpanel.com.log combined
   AllowOverride      All
   <Directory ./home/testuser/html/admin.openpanel.com>
       AddType text/plain .pl
   </Directory>
</VirtualHost>
