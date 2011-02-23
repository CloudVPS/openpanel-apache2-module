
-- Nu huries, we can wait just fine
.timeout 2000


-- Generate new https objects, parented to the httpsvhost objects already in the DB
INSERT INTO Objects (uuid,metaid,parent,owner,uniquecontext,class,content)
SELECT 
    substr(uuid,0,25) || lower(hex(randomblob(6))) as uuid, -- generate a new uid (the cheap way)
    'https' as metaid, -- https objec
    id as parent, -- child of the vhost it is derived from
    owner as owner, -- same owner as parent object
    id as uniquecontext, -- unique in parent
    (SELECT id FROM objects WHERE uuid = '01E3D712-1E45-11E0-9A5E-BEF9DED72045') as class, -- https
    content -- simply copy the content.
FROM Objects
WHERE
    class = (SELECT id FROM objects WHERE uuid = '01E3D712-1E45-11E0-9A5E-BEF9DED72085'); -- httpsvhost
    
-- Change the class for all httpsvhosts.
UPDATE Objects
SET 
    class = (SELECT id FROM objects WHERE uuid = '59417a58-d793-4ec7-a909-a9acb0236971') -- vhost
WHERE
    class = (SELECT id FROM objects WHERE uuid = '01E3D712-1E45-11E0-9A5E-BEF9DED72085'); -- httpsvhost

