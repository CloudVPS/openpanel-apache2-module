import random, time, string

from OpenPanel.exception import CoreException

requires=['Domain']

pemtemplate = """
-----BEGIN CERTIFICATE-----
MIIBpzCCARACCQCuuYZQEjDNXzANBgkqhkiG9w0BAQUFADAYMRYwFAYDVQQDEw12%s
YWdyYW50dXAuY29tMB4XDTExMDUxNDA3NTMyNFoXDTIxMDUxMTA3NTMyNFowGDEW
MBQGA1UEAxMNdmFncmFudHVwLmNvbTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkC
gYEAzmxy0aEJR7n9mcTURAWGIFQxmBtqGbbN719p1t8IF6Vcl0oCzWV/vO4BvnGY
hJBKkPhIbTzqKVP/DoBU9H82Lyne7eIQEtdpCpHrq+af4PZXb68lDQsopTmMZxpY
P70awpD5dOu/LATFdT6yT1LBqNL6TY9Oa2CVEyAHBPrgI2ECAwEAATANBgkqhkiG
9w0BAQUFAAOBgQAb0kB72nFooA2xpnDsQbAnppTlhenP6dQdExebztNwFfpvnmQY
/YZNtnxTR97LPyNWc2bXEPfWpAmaCCp3FZwRt+qI34pi6AHVQmL0YkkUVGbr6lKg
8nJg4aJWHao041w7W1nabcV3bF116x8YT1LR2OclBqj7mHKO+rs6ObwJ4w==
-----END CERTIFICATE-----
-----BEGIN RSA PRIVATE KEY-----
MIICWwIBAAKBgQDObHLRoQlHuf2ZxNREBYYgVDGYG2oZts3vX2nW3wgXpVyXSgLN
ZX+87gG+cZiEkEqQ+EhtPOopU/8OgFT0fzYvKd7t4hAS12kKkeur5p/g9ldvryUN
CyilOYxnGlg/vRrCkPl0678sBMV1PrJPUsGo0vpNj05rYJUTIAcE+uAjYQIDAQAB
AoGARt87KLgWfJbh+Dp4+oW/ADiH8oZvDjcSS35B03Et0RiISrUxcx3HFrHMzXso
91KnF1D3ZPIGH3xkVLFA+dT3X+lNy2sdcFworHIyDybjARbl5bi9UFKSQgL6ysX5
eLlN34oCx8oxyhcCcHqYy7ODB1dp0uMb185fi9TkYTjdcXECQQD/lglNnVV5t2QH
y3NkB63+WveHQ/4K3OLByZsfk5apzXJbBl88YLCzd6k4Ht8k681g9sJT2SiXZzRb
YLwtAhxFAkEAzsIHpUnGsEUSAsDaPRmyCuU38lNvAP6mNDdfAr2qt13qH7Ek8TbR
OssrQMdn++F/cZdMDhar1Gp5fnvHEPxSbQJAW4G0Qv2ZpMJsJf5Jif7V5MNc7CkD
w4/h/8/Woi+NOa48yyo03sEfgqHNDDLBYc6q8ZRnbu3JAqwIqs6vkM24oQJAO/Xt
PbQdmPUO+hlvsRSIm7xJ807a/FDdZntAZI0Yu0DTKwm2ivW4y/IrwweRUmzdBtY3
ykYs4tdWEKZY3sYuKQJAFC18PR3TT9RzL2GKuEE4tPRmMGMs8L87cIoq6ljgu+sT
PclfJa6E2Q4OtmjQWcF7kVZbBbQ01RbQtJD4ajKKdw==
-----END RSA PRIVATE KEY-----
"""

def test(ctx):
    try:
        ctx.conn.rpc.classinfo(classid='Domain:Vhost')
    except CoreException:
        ctx.fail("class-not-found", 'Domain:Vhost class not found, skipping tests')
        return

    vhost = "www."+ctx.domain
    vhostuuid = ctx.conn.rpc.create(classid="Domain:Vhost", objectid=vhost, parentid=ctx.domainuuid)['body']['data']['objid']
    ctx.logger.debug('created Domain:Vhost %s (%s)' % (vhost, vhostuuid))

    sslvhostuuid = ctx.conn.rpc.create(classid="Domain:HTTPS", parentid=vhostuuid, data=dict(pem=pemtemplate % '', ip='127.0.0.1'))['body']['data']['objid']
    ctx.logger.debug('created Domain:HTTPS %s (%s)' % (vhost, sslvhostuuid))

    ctx.conn.rpc.delete(classid="Domain:HTTPS", objectid=sslvhostuuid)

    vhost2 = "www2."+ctx.domain
    vhost2uuid = ctx.conn.rpc.create(classid="Domain:Vhost", objectid=vhost2, parentid=ctx.domainuuid)['body']['data']['objid']
    ctx.logger.debug('created Domain:Vhost %s (%s)' % (vhost2, vhost2uuid))

    success = False
    try:
    	sslvhost2uuid = ctx.conn.rpc.create(classid="Domain:HTTPS", parentid=vhost2uuid, data=dict(pem=pemtemplate % 'GARBAGE', ip='127.0.0.1'))['body']['data']['objid']
    except CoreException:
        success=True

    if not success:
        ctx.fail("https-invalid-pem", "module accepted invalid PEM file")
        return False

    ctx.logger.debug('invalid PEM file correctly refused')

    # if not smtpauthcheck(ctx, box, ctx.password):
    #     ctx.fail("smtp-auth", 'smtp auth does not work')
    #     return False
    # 
    # token = ''.join(random.choice(string.letters) for i in xrange(32)).lower()
    # msg = roundtrip(ctx, token, "no-retr")
    # if token == msg[-1]:
    #     ctx.logger.debug('token email sent and received correctly')
    # else:
    #     ctx.fail("token-compare", "sent (%s) got (%s)" % (token, msg[-1]))
    # 
    # aliasuuid = ctx.conn.rpc.create(classid="Mail:Alias", objectid=alias, parentid=mailuuid, data=dict(pridest='test@example.com'))['body']['data']['objid']
    # ctx.conn.rpc.delete(classid="Mail:Alias", objectid=aliasuuid)
    # 
    # if postmapq('virtual_alias', alias)[1]:
    #     ctx.fail("not-deleted", 'create+delete of alias leaves entry behind')
    #     return False
    # 
    # return True

def cleanup(ctx):
    # Domain will clean it all up
    pass
