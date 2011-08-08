#!/usr/bin/haserl
<?
	echo "Content-type: text/plain"
	echo ""
	/bin/cat /www/oem.sh |/bin/grep -v "/bin/sh"
?>
