#!/usr/bin/haserl -u
<?

. conf/conf.sh

act=$FORM_act
uploadfile=$FORM_uploadfile


case "$act" in
	backup)
		echo "Content-type: application/octet-stream"
		echo "Content-Disposition: attachment; filename=\"`hostname`-`date +%Y%m%d%H%M%S`.cfg\""
		echo 

		tar c - /etc/kdb /etc/kdb.md5 /etc/eocd/*
	;;
	restore)
		echo "Content-type: text/html"
		echo 
		echo "<html><body>"
		echo "<h2>"

		if [ -r $uploadfile ]; then
			if tar x -C /tmp -f $uploadfile; then
				# check MD5 of KDB
				kdb_md5=$(md5sum /tmp/etc/kdb |awk '{ print $1 }')
				if [ "$kdb_md5" != "$(cat /tmp/etc/kdb.md5)" ]; then
					echo "Backup file is corrupted"
				else
					tar x -C / -f $uploadfile
					echo "File imported successfully"
				fi
				rm -rf /tmp/etc
				echo "<br><br><b>Device is rebooting.</b>"
				/sbin/reboot
			else
				echo "Error occured while import configuration"
			fi
			rm -f $uploadfile
		else
			echo "File '$uploadfile' not found "
		fi
	;;
	default)
		echo "Content-type: text/html"
		echo 
		
		# remove KDB files. they will be restored from kdb.default on boot by /etc/init.d/kdb script
		rm /etc/kdb
		rm /etc/kdb.md5
		
		# Restore eocd state
		if [ -f /etc/eocd/eocd.conf ]; then
		    cp /etc/eocd/eocd.conf.default /etc/eocd/eocd.conf
		    killall -HUP eocd
		fi
		echo "<html><body>"
		echo "<h2>Default configuration restored"
		echo "<br><br><b>Device is rebooting.</b>"
		echo "<br><br><a href='/wf2/'>Open web-interface</a>"
		/sbin/reboot
	;;
esac
		
?>
