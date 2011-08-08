#!/usr/bin/haserl -u
<?
	echo "Content-type: text/html"
	echo
	echo "<html><body>"

	path=$FORM_path
	uploadfile=$FORM_uploadfile

	if [ -r $uploadfile ]; then
		/bin/mv $uploadfile $path/$FORM_uploadfile_name && \
echo "File '$FORM_uploadfile_name' uploaded successfully (uploaded $CONTENT_LENGTH bytes)." || \
echo "Error moving file '$FORM_uploadfile_name' to '$path/$FORM_uploadfile_name'"
	else
		echo "File '$uploadfile' not found"
	fi

	echo "<br><br>"
	echo "<a href='$HTTP_REFERER'>Return to web-interface</a>"
	echo "</body></html>"
?>
