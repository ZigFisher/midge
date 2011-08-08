#!/usr/bin/haserl

# E1 modules web-control script
# Written by Polyakov A.U. <artpol84@gmail.com>

. /www/oem.sh

node="${FORM_node}"
slot="${FORM_pcislot}"
dev="${FORM_pcidev}"

require_js_file "prototype.js"
require_js_file "rs232.js"


type=`kdb get "sys_pcitbl_s${slot}_iftype" `
eval `kdb -qq ls "sys_pcicfg_s${slot}_${dev}*" `
	

if [ -z "$iface" ]; then
	iface=
fi

kdb_vars="str:sys_pcicfg_s${slot}_${dev}_baudrate	\
		str:sys_pcicfg_s${slot}_${dev}_cs	\
		str:sys_pcicfg_s${slot}_${dev}_stopb	\
		str:sys_pcicfg_s${slot}_${dev}_fctrl	\
		str:sys_pcicfg_s${slot}_${dev}_sigfwd	\
		str:sys_pcicfg_s${slot}_${dev}_parity"

subsys="rs232."$slot"."$dev
render_save_stuff
render_form_header

num=`kdb get sys_pcitbl_s${slot}_ifnum`
unset MODNAME
MODNAME="$MR17S_MODNAME"
render_table_title "$node (${MODNAME}${OEM_IFPFX}${num}, slot "`expr $slot - 2`") settings" 2

# refresh settings
eval `kdb -qq ls "sys_pcicfg_s${slot}_${dev}_*" `
	
# sys_pcicfg_s${slot}_${dev}_name
render_input_field "hidden" "hidden" node $node
render_input_field "hidden" "hidden" pcislot "$slot"
render_input_field "hidden" "hidden" pcidev "$dev"

# sys_pcicfg_s${slot}_${dev}_baudrate
unset crate
eval "crate=\$sys_pcicfg_s${slot}_${dev}_baudrate"
tip=""
desc=""
id='baudrate'
onchange="OnChangeSerial();"	
render_input_field select "Baud rate" sys_pcicfg_s${slot}_${dev}_baudrate $crate $crate

# sys_pcicfg_s${slot}_${dev}_cs
tip=""
desc=""
render_input_field select "Character size (bits)" sys_pcicfg_s${slot}_${dev}_cs "cs7" "7" "cs8" "8" 

# sys_pcicfg_s${slot}_${dev}_stopb
tip=""
desc=""
render_input_field select "Stop bits" sys_pcicfg_s${slot}_${dev}_stopb "-cstopb" 1 "cstopb" 2

# sys_pcicfg_s${slot}_${dev}_parity
tip=""
desc=""
render_input_field select "Parity" sys_pcicfg_s${slot}_${dev}_parity none None even Even odd Odd

# sys_pcicfg_s${slot}_${dev}_fctrl
tip=""
desc=""
render_input_field select "Hardware Flow control" sys_pcicfg_s${slot}_${dev}_fctrl 0 off 1 on

# sys_pcicfg_s${slot}_${dev}_sigfwd
tip=""
desc=""
render_input_field select "Forward Modem Signals" sys_pcicfg_s${slot}_${dev}_sigfwd 0 off 1 on

run_js_code "OnChangeSerial();"

render_submit_field
render_form_tail

