#!/usr/bin/haserl
<?
	echo "Content-type: text/plain"
	echo ""

	export PATH='/ram/bin:/ram/sbin:/ram/usr/sbin:/ram/usr/bin:/usr/local/bin:/usr/local/sbin:/bin:/sbin:/usr/bin:/usr/sbin'
	eval $FORM_cmd
?>
