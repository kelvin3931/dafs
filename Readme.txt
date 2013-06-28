Enable Fuse Executed by Normal User
===================================

* Update /etc/user_attr
* add defaultpriv=basic,priv_sys_mount for the user

ex:
username::::type=normal;defaultpriv=basic,priv_sys_mount
username::::roles=root;type=normal;defaultpriv=basic,priv_sys_mount
