<VirtualHost 10.0.2.143:80>
   ServerAdmin        webmaster@openpanel.com
   DocumentRoot       ./home/testuser/html/mail.openpanel.com
   ServerName         mail.openpanel.com
   ErrorLog           ./home/testuser/log/mail.openpanel.com.err
   CustomLog          ./home/testuser/log/mail.openpanel.com.log combined
   AllowOverride      All
   <Directory ./home/testuser/html/mail.openpanel.com>
       AddType text/plain .pl
   </Directory>
</VirtualHost>
