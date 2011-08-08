#!/bin/sh
# (c) Vladislav Moskovets 2005
#

[ "$refresh" ] && echo "Refresh: $refresh;url=$refresh_url"
[ -n "$FORM_SESSIONID" ] && SESSIONID=$FORM_SESSIONID
echo "Set-Cookie: SESSIONID=$SESSIONID; path=/;"
echo 'Cache-Control: no-cache
Content-Type: text/html; Charset=utf-8

'
echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title>WebFace</title>
<script type="text/javascript">function overlib(){return true;}</script>
<script type="text/javascript" src="/js/overlib.js"></script>
<script type="text/javascript" src="/js/overlib_bubble.js"> </script>
<script type="text/javascript" src="/js/script_tmt_validator.js"> </script>
<script type="text/javascript" src="/js/vlad_tmt_validator.js"> </script>
<script type="text/javascript" src="/js/baloon_validator.js"></script>
<script type="text/javascript" src="/js/misc.js"> </script>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link href="css/content.css" rel="stylesheet" type="text/css">
<link rel="shortcut icon" href="favicon.ico" >
</head>

<body link="#0000CC" vlink="#0000CC" alink="#0000CC">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>
<table width="750" border="0" cellspacing="0" cellpadding="2">
  <tr valign="bottom"> 
    <td width="100" height="51" align="center" valign="middle"> <strong><a href="http://sigrand.ru" target="_blank"><img src="img/logo.jpg" height="51" border="0"></a></strong></td>
    <td width="650" height="51"><img src="img/header_bg.png" height="100" width="650" border="0"></td>
  </tr>
  <tr valign="top"> 
    <td width="150" bgcolor="#9D9D9D">
		<table width="100%" border="0" cellpadding="6" cellspacing="0">
        <tr>'
. common/menu.sh

echo '	</td>
        </tr>
		</table></td>
    <td width="600"><table width="100%" border="0" cellpadding="10" cellspacing="0">
        <tr><td><br /> <br />' 
		
