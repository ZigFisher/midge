#!/usr/bin/haserl

# E1 modules web-control script
# Written by Polyakov A.U. <artpol84@gmail.com>

. /www/oem.sh

iface="${FORM_iface}"
slot="${FORM_pcislot}"
dev="${FORM_pcidev}"

type=`kdb get "sys_pcitbl_s${slot}_iftype" `
eval `kdb -qq ls "sys_pcicfg_s${slot}_${dev}*" `

# Check iface to be HDLC capable
muxpath="/sys/class/net/$iface/hw_private/muxonly"
unset muxonly
if [ -f "$muxpath" ]; then
	muxonly=`cat $muxpath`
else
	muxonly=0
fi

if [ -n "$iface" ]; then
	kdb_vars=""
	if [ "$muxonly" = "0" ]; then
		kdb_vars="${kdb_vars} \
			str:sys_pcicfg_s${slot}_${dev}_proto	\
			str:sys_pcicfg_s${slot}_${dev}_enc	\
			str:sys_pcicfg_s${slot}_${dev}_parity	\
			int:sys_pcicfg_s${slot}_${dev}_int	\
			int:sys_pcicfg_s${slot}_${dev}_to	\
			int:sys_pcicfg_s${slot}_${dev}_lcode	\
			int:sys_pcicfg_s${slot}_${dev}_hcrc	\
			int:sys_pcicfg_s${slot}_${dev}_fill	\
			bool:sys_pcicfg_s${slot}_${dev}_clk \
			int:sys_pcicfg_s${slot}_${dev}_inv	"
	fi

	kdb_vars="${kdb_vars} \
		  	bool:sys_pcicfg_s${slot}_${dev}_fram	\
			bool:sys_pcicfg_s${slot}_${dev}_lhaul	\
			int:sys_pcicfg_s${slot}_${dev}_llpb	\
			int:sys_pcicfg_s${slot}_${dev}_rlpb "

	fram=`kdb get sys_pcicfg_s${slot}_${dev}_fram`	
#	echo "FRAM=$fram<br>"
	if [ "$fram" -eq "1" ]; then
		kdb_vars="${kdb_vars} \
			bool:sys_pcicfg_s${slot}_${dev}_crc4	\
			bool:sys_pcicfg_s${slot}_${dev}_cas		\
			bool:sys_pcicfg_s${slot}_${dev}_ts16 "
		if [ "$muxonly" = "0" ]; then
			kdb_vars="${kdb_vars} \
				str:sys_pcicfg_s${slot}_${dev}_smap"
		fi
	fi

#	echo "kdb_vars=$kdb_vars<br>"

	subsys="e1."$slot"."$dev
	render_save_stuff

	render_form_header

	num=`kdb get sys_pcitbl_s${slot}_ifnum`
    unset MODNAME
    case "$type" in
    "$MR16G_DRVNAME")
        MODNAME="$MR16G_MODNAME"
        ;;
    "$MR17G_DRVNAME")
        MODNAME="$MR17G_MODNAME"
        ;;
    esac
    
    
	render_table_title "$iface (${MODNAME}${OEM_IFPFX}${num}, slot "`expr $slot - 2`") settings" 2

	# Seems that config script ends later than this code
	# So need delay because config script can change smap
	sleep 1	
	# refresh settings
	eval `kdb -qq ls "sys_pcicfg_s${slot}_${dev}_*" `
	
	# Check for correctness of smap value
	
	# sys_pcicfg_s${slot}_${dev}_name
	render_input_field "hidden" "hidden" iface $iface
	render_input_field "hidden" "hidden" pcislot "$slot"
	render_input_field "hidden" "hidden" pcidev "$dev"

	if [ "$muxonly" = "0" ]; then

		# sys_pcicfg_s${slot}_${dev}_proto
		tip=""
		desc=""
		render_input_field select "HDLC protocol" sys_pcicfg_s${slot}_${dev}_proto  hdlc HDLC hdlc-eth ETHER-HDLC cisco CISCO-HDLC fr FR ppp PPP x25 X25

	#TODO:	1. Make in Java script

	#	2. find out what options is for FR!
		proto=`kdb get sys_pcicfg_s${slot}_${dev}_proto`
	    case "$proto" in
		hdlc*)
		    # sys_pcicfg_s${slot}_${dev}_enc
		    encodings="nrz nrzi fm-mark fm-space manchester"
	    	tip=""
		    desc=""
		    render_input_field select "Encoding" sys_pcicfg_s${slot}_${dev}_hdlc_enc $(for i in $encodings; do echo $i $i;done)

	    	# sys_pcicfg_s${slot}_${dev}_parity
		    parity="crc16-itu no-parity crc16 crc16-pr0 crc16-itu-pr0 crc32-itu"
		    tip=""
	    	desc=""
		    render_input_field select "Parity" sys_pcicfg_s${slot}_${dev}_hdlc_parity $(for i in $parity; do echo $i $i;done)
		    ;;	    
		cisco)
    		# sys_pcicfg_s${slot}_${dev}_int
	    	default=10
	        tip=""
    	    render_input_field select "Interval" sys_pcicfg_s${slot}_${dev}_cisco_int $(for i in `seq 1 10`; do n=$(($i*10)); echo $n $n; done)
	    
    		# sys_pcicfg_s${slot}_${dev}_to
		    default=25
	    	tip=""
        	render_input_field select "Timeout" sys_pcicfg_s${slot}_${dev}_cisco_to  $(for i in `seq 1 20`; do n=$(($i*5)); echo $n $n; done)
		    ;;
		*)
	    	;;
		esac
	fi
	# sys_pcicfg_s${slot}_${dev}_fram
    tip=""
	desc="check to enable"
    render_input_field checkbox "E1 framed mode" sys_pcicfg_s${slot}_${dev}_fram

	# TODO: Java-script?
	# Valid only in framed mode
	fram=`kdb get sys_pcicfg_s${slot}_${dev}_fram`	
	if [ "$fram" -eq "1" ]; then

       	# sys_pcicfg_s${slot}_${dev}_ts16
	    tip=""
	    desc="check to use"
       	render_input_field checkbox "Use time slot 16" sys_pcicfg_s${slot}_${dev}_ts16


		if [ "$muxonly" = "0" ]; then
	   		# sys_pcicfg_s${slot}_${dev}_smap
		    tip=""
	   		desc="example: 2-3,6-9,15-20"
		    render_input_field text "Slotmap" sys_pcicfg_s${slot}_${dev}_smap
		fi
	    
		# sys_pcicfg_s${slot}_${dev}_crc4
    	tip=""
	   	desc="check to enable"
	    render_input_field checkbox "E1 CRC4 multiframe" sys_pcicfg_s${slot}_${dev}_crc4

	   	# sys_pcicfg_s${slot}_${dev}_cas
	   	tip=""
   		desc="check to enable"
	   	render_input_field checkbox "E1 CAS multiframe" sys_pcicfg_s${slot}_${dev}_cas
	fi

	if [ "$muxonly" = "0" ]; then
		# sys_pcicfg_s${slot}_${dev}_clk
    	tip=""
		desc="check to enable"
		render_input_field checkbox "E1 external transmit clock" sys_pcicfg_s${slot}_${dev}_clk
	fi
	
    # sys_pcicfg_s${slot}_${dev}_lhaul
    tip=""
    desc="check to enable"
    render_input_field checkbox "E1 long haul mode" sys_pcicfg_s${slot}_${dev}_lhaul

    # sys_pcicfg_s${slot}_${dev}_lcode
    tip=""
    desc=""
    render_input_field select "E1 HDB3/AMI line code" sys_pcicfg_s${slot}_${dev}_lcode  1 HDB3 0 AMI

	if [ "$muxonly" = "0" ]; then
	    # sys_pcicfg_s${slot}_${dev}_crc32
    	tip=""
	    desc="Select HDLC CRC length"
    	render_input_field select "CRC" sys_pcicfg_s${slot}_${dev}_hcrc 0 CRC32 1 CRC16
				
	    # sys_pcicfg_s${slot}_${dev}_fill
    	tip=""
	    desc="Select HDLC fill byte value"
    	render_input_field select "Fill" sys_pcicfg_s${slot}_${dev}_fill  0 FF 1 7E
							
	    # sys_pcicfg_s${slot}_${dev}_inv
    	tip=""
	    desc="Select HDLC inversion mode"
    	render_input_field select "Inversion" sys_pcicfg_s${slot}_${dev}_inv  0 off 1 on
	fi

	# sys_pcicfg_s${slot}_${dev}_llpb
    tip=""
	desc="Enable E1 Local Loopback"
    render_input_field checkbox "Local Loopback" sys_pcicfg_s${slot}_${dev}_llpb

	# sys_pcicfg_s${slot}_${dev}_rlpb
    tip=""
    desc="Enable E1 Remote Loopback"
    render_input_field checkbox "Remote Loopback" sys_pcicfg_s${slot}_${dev}_rlpb

    render_submit_field
    render_form_tail

fi
