#!/usr/bin/python
import sys, subprocess, os

certs=[]
keys=[]

def openssl(cmd, input):
	p = subprocess.Popen(["/usr/bin/openssl"] + cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=open(os.devnull,'w'))
	p.stdin.write('\n'.join(input))
	p.stdin.close()
	output = p.stdout.read()
	if p.wait():
		sys.exit(1)
	return output

def verifycert(cert):
	global certs
	modulus = openssl(['x509','-noout','-modulus'], cert)
	certs.append(modulus)

def verifykey(key):
	global keys
	modulus = openssl(['rsa','-noout', '-passin','pass:','-modulus'], key)
	keys.append(modulus)

def tryverify(pem):
	if pem and pem[0] == "-----BEGIN CERTIFICATE-----":
		verifycert(cur)
	if pem and pem[0] == "-----BEGIN RSA PRIVATE KEY-----":
		verifykey(cur)

f = open(sys.argv[1])


cur=[]
for l in f:
	l = l.strip()
	if l.startswith("-----BEGIN"):
		tryverify(cur)
		cur = []

	cur.append(l)

tryverify(cur)

if not certs:
	sys.exit(1)

if len(keys) != 1:
	sys.exit(1)

if keys[0] not in certs:
	sys.exit(1)
