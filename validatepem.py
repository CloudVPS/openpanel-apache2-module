#!/usr/bin/python
import sys, subprocess, os

def openssl(cmd, input):
	p = subprocess.Popen(["/usr/bin/openssl"] + cmd, stdin=subprocess.PIPE, stderr=open(os.devnull,'w'))
	p.stdin.write('\n'.join(input))
	p.stdin.close()
	if p.wait():
		sys.exit(1)

def verifycert(cert):
	openssl(['x509','-noout'], cert)

def verifykey(key):
	openssl(['rsa','-noout', '-passin','pass:'], key)

def tryverify(pem):
	if pem and pem[0] == "-----BEGIN CERTIFICATE-----":
		verifycert(cur)
	if pem and pem[0] == "-----BEGIN RSA PRIVATE KEY-----":
		verifykey(cur)

f = open(sys.argv[1])

certs=[]
keys=[]

cur=[]
for l in f:
	l = l.strip()
	if l.startswith("-----BEGIN"):
		tryverify(cur)
		cur = []

	cur.append(l)

tryverify(cur)
