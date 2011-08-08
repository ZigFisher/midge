function mr16gModuleName(pcislot) {
	return $.sprintf("%s%s%s", config.getOEM("MR16G_MODNAME"), config.getOEM("OEM_IFPFX"),
			config.get($.sprintf("sys_pcitbl_s%s_ifnum", pcislot)));
}

function mr17gModuleName(pcislot) {
	return $.sprintf("%s%s%s", config.getOEM("MR17G_MODNAME"), config.getOEM("OEM_IFPFX"),
			config.get($.sprintf("sys_pcitbl_s%s_ifnum", pcislot)));
}

Controllers.e1 = function(iface, pcislot, pcidev) {
	var page = this.Page();
	page.setHelpPage("e1");
	page.setSubsystem($.sprintf("e1.%s.%s", pcislot, pcidev));

	page.addTab({
		"id": "e1",
		"name": "E1",
		"func": function() {
			var c, field, id;
			var options = {};

			c = page.addContainer("e1");

			/* set title */
			var moduleType = config.get($.sprintf("sys_pcitbl_s%s_iftype", pcislot));
			var moduleName = (moduleType == config.getOEM("MR16G_DRVNAME")) ?
					mr16gModuleName(pcislot) : mr17gModuleName(pcislot);
			c.addTitle($.sprintf("%s (%s, %s, %s %s) %s", iface, moduleName,
				config.getCachedOutput($.sprintf("muxonly_%s", iface)) == 0
				? _("full capabilities") : _("multiplexing only"), _("slot"), pcislot - 2,
				_("settings")));

			/* IDs of *HDLC widgets */
			var hdlcWidgetsIDs = new Array();

			/* IDs of CISCO widgets */
			var ciscoWidgetsIDs = new Array();

			/* IDs of framed widgets */
			var framedWidgetsIDs = new Array();

			/* add, if not exist, widgets for HDLC and ETHER-HDLC protocol */
			var addHdlcWidgets = function() {
				if (hdlcWidgetsIDs.length != 0) return;

				/* widget after which insert this element */
				var protoWidget = $("#pcicfgProto").parents("tr");

				/* encoding */
				field = {
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_hdlc_enc", pcislot, pcidev),
					"id": "enc",
					"text": "Encoding",
					"options": "nrz nrzi fm-mark fm-space manchester"
				};
				c.addWidget(field, {"type": "insertAfter", "anchor": protoWidget});
				hdlcWidgetsIDs.push("enc");

				/* parity */
				field = {
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_hdlc_parity", pcislot, pcidev),
					"id": "parity",
					"text": "Parity",
					"options": "crc16-itu no-parity crc16 crc16-pr0 crc16-itu-pr0 crc32-itu"
				};
				c.addWidget(field, {"type": "insertAfter", "anchor": protoWidget});
				hdlcWidgetsIDs.push("parity");
			};

			/* add, if not exist, widgets for CISCO protocol */
			var addCiscoWidgets = function() {
				if (ciscoWidgetsIDs.length != 0) return;

				/* widget after which insert this element */
				var protoWidget = $("#pcicfgProto").parents("tr");

				/* interval */
				var interval;
				for (var i = 1; i <= 10; i++) {
					if (interval) interval += " " + i * 10;
					else interval = "" + i * 10;
				}
				field = {
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_cisco_int", pcislot, pcidev),
					"id": "cisco_int",
					"text": "Interval",
					"options": interval,
					"defaultValue": "10"
				};
				c.addWidget(field, {"type": "insertAfter", "anchor": protoWidget});
				ciscoWidgetsIDs.push("cisco_int");

				/* timeout */
				var to;
				for (var i = 1; i <= 20; i++) {
					if (to) to += " " + i * 5;
					else to = "" + i * 5;
				}
				field = {
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_cisco_to", pcislot, pcidev),
					"id": "cisco_to",
					"text": "Timeout",
					"options": to,
					"defaultValue": "25"
				};
				c.addWidget(field, {"type": "insertAfter", "anchor": protoWidget});
				ciscoWidgetsIDs.push("cisco_to");
			};

			/* add, if not exist, widgets for framed mode */
			var addFramedWidgets = function() {
				var field;
				if (framedWidgetsIDs.length != 0) return;

				/* widget after which insert this element */
				var framWidget = $("#fram").parents("tr");

				/* add SMAP only for full capable interfaces */
				if (config.getCachedOutput($.sprintf("muxonly_%s", iface)) == 0) {
					var id = $.sprintf("sys_pcicfg_s%s_%s_smap", pcislot, pcidev);
					field = {
						"type": "text",
						"name": id,
						"id": id,
						"text": "Slotmap",
						"descr": "example: 2-3,6-9,15-20",
						"validator": {"smap": true}
					};
					c.addWidget(field, {"type": "insertAfter", "anchor": framWidget});
					framedWidgetsIDs.push(id);

					/* subsystem can change slotmap value, so after request is performed, update it */
					options.onSuccess = function() {
						updateFields(id, true);
					};
				}

				field = {
					"type": "checkbox",
					"name": $.sprintf("sys_pcicfg_s%s_%s_crc4", pcislot, pcidev),
					"id": "crc4",
					"text": "E1 CRC4 multiframe"
				};
				c.addWidget(field, {"type": "insertAfter", "anchor": framWidget});
				framedWidgetsIDs.push("crc4");

				field = {
					"type": "checkbox",
					"name": $.sprintf("sys_pcicfg_s%s_%s_ts16", pcislot, pcidev),
					"id": "ts16",
					"text": "Use time slot 16",
					"onClick": onTS16Change,
					"defaultState": "checked"
				};
				c.addWidget(field, {"type": "insertAfter", "anchor": framWidget});
				framedWidgetsIDs.push("ts16");
			};

			/* remove, if exist, specified widgets */
			var removeWidgets = function(widgetsIDs) {
				if (widgetsIDs.length != 0) {
					$.each(widgetsIDs, function(num, value) {
						$("#" + value).parents("tr").remove();
					});

					/* remove all IDs from array */
					widgetsIDs.splice(0, widgetsIDs.length);
				}
			};

			/* handler to call when FRAM option changes */
			var onFramChange = function() {
				/* cancel updating for SMAP */
				delete options.onSuccess;

				/* remove all dynamic widgets */
				removeWidgets(framedWidgetsIDs);

				/* add widgets for framed mode */
				if ($("#fram").attr("checked")) {
					addFramedWidgets();
					onTS16Change();
				}
			};

			/* handler to call when TS16 options changes */
			var onTS16Change = function() {
				/* remove CAS options */
				$("#cas").parents("tr").remove();

				/* if TS16 option is not active, add CAS option */
				if (! $("#ts16").attr("checked")) {
					var field = {
						"type": "checkbox",
						"name": $.sprintf("sys_pcicfg_s%s_%s_cas", pcislot, pcidev),
						"id": "cas",
						"text": "E1 CAS multiframe"
					};
					c.addWidget(field, {"type": "insertAfter", "anchor": $("#ts16").parents("tr")});
				}
			};

			/* handler to call when PCICFG PROTO option changes */
			var onPcicfgProtoChange = function() {
				/* remove all dynamic widgets */
				removeWidgets(hdlcWidgetsIDs);
				removeWidgets(ciscoWidgetsIDs);

				switch ($("#pcicfgProto").val()) {
					/* add widgets for *HDLC protocol */
					case "hdlc":
					case "hdlc-eth":
						addHdlcWidgets();
						break;
					/* add widgets for CISCO protocol */
					case "cisco":
						addCiscoWidgets();
						break;
				}

				/* set network interface proto */
				$("#ifaceProto").val($("#pcicfgProto").val() == "hdlc-eth" ? "ether" : "hdlc");
			};

			/* add widgets that always present */

			field = {
				"type": "checkbox",
				"name": $.sprintf("sys_mux_%s_mxen", iface),
				"text": "Enable multiplexing",
				"descr": "Enable multiplexing on this interface",
				"tip": "This option is equivalent to MXEN on a multiplexing page."
			};
			c.addWidget(field);

			field = {
				"type": "checkbox",
				"name": $.sprintf("sys_pcicfg_s%s_%s_fram", pcislot, pcidev),
				"id": "fram",
				"text": "E1 framed mode",
				"onClick": onFramChange
			};
			c.addWidget(field);

			field = {
				"type": "checkbox",
				"name": $.sprintf("sys_pcicfg_s%s_%s_lhaul", pcislot, pcidev),
				"text": "E1 long haul mode"
			};
			c.addWidget(field);

			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_lcode", pcislot, pcidev),
				"text": "E1 HDB3/AMI line code",
				"options": {"1": "HDB3", "0": "AMI"}
			};
			c.addWidget(field);

			field = {
				"type": "checkbox",
				"name": $.sprintf("sys_pcicfg_s%s_%s_llpb", pcislot, pcidev),
				"text": "Local Loopback",
				"descr": "Enable E1 Local Loopback"
			};
			c.addWidget(field);

			field = {
				"type": "checkbox",
				"name": $.sprintf("sys_pcicfg_s%s_%s_rlpb", pcislot, pcidev),
				"text": "Remote Loopback",
				"descr": "Enable E1 Remote Loopback"
			};
			c.addWidget(field);

			/* add widgets for full capable interface */
			if (config.getCachedOutput($.sprintf("muxonly_%s", iface)) == 0) {
				/* protocol select */
				field = {
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_proto", pcislot, pcidev),
					"id": "pcicfgProto",
					"text": "HDLC protocol",
					"onChange": onPcicfgProtoChange,
					"options": {
						"hdlc": "HDLC", "hdlc-eth": "ETHER-HDLC", "cisco": "CISCO-HDLC", "fr": "FR",
						"ppp": "PPP", "x25": "X25"
					}
				};
				c.addWidget(field);

				field = {
					"type": "checkbox",
					"name": $.sprintf("sys_pcicfg_s%s_%s_clk", pcislot, pcidev),
					"text": "E1 external transmit clock"
				};
				c.addWidget(field);

				field = {
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_hcrc", pcislot, pcidev),
					"text": "CRC",
					"descr": "Select HDLC CRC length",
					"options": {"0": "CRC32", "1": "CRC16"}
				};
				c.addWidget(field);

				field = {
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_fill", pcislot, pcidev),
					"text": "Fill",
					"descr": "Select HDLC fill byte value",
					"options": {"0": "FF", "1": "7E"}
				};
				c.addWidget(field);

				field = {
					"type": "select",
					"name": $.sprintf("sys_pcicfg_s%s_%s_inv", pcislot, pcidev),
					"text": "Inversion",
					"descr": "Select HDLC inversion mode",
					"options": {"0": "off", "1": "on"}
				};
				c.addWidget(field);

				/*
				 * This key (interface protocol) is set in the /etc/init.d/e1, but we need
				 * it in web before current request will be completed.
				 */
				field = {
					"type": "hidden",
					"name": $.sprintf("sys_iface_%s_proto", iface),
					"id": "ifaceProto",
					"defaultValue":
						$("#pcicfgProto").val() == "hdlc-eth" ? "ether" : "hdlc"
				};
				c.addWidget(field);
			}

			c.addSubmit(options);

			onFramChange();
			onTS16Change();
			onPcicfgProtoChange();
		}
	});

	page.generateTabs();
};

Controllers.linkdeps = function() {
	var page = this.Page();
	page.setSubsystem("linkdeps");

	page.addTab({
		"id": "linkdeps",
		"name": "Linkdeps",
		"func": function() {
			var c, field;
			c = page.addContainer("linkdeps");

			var list = c.createList({
				"tabId": "linkdeps",
				"header": ["Link-master", "Link-slave"],
				"varList": ["link_master", "link_slave"],
				"listItem": "sys_linkdeps_",
				"addMessage": "Add link dependency",
				"editMessage": "Edit link dependency",
				"listTitle": "Link dependency"
			});

			field = {
				"type": "select",
				"name": "link_master",
				"text": "Link-master",
				"descr": "If link of this interface is down, link of link-slave interface will be down too.",
				"options": function() {
					var ifaces = [];

					/* only SHDSL interfaces can be link-master */
					$(config.getParsed("sys_ifaces")).each(function(name, iface) {
						if (iface.search(/dsl/) != -1) {
							ifaces.push(iface);
						}
					});

					return ifaces;
				}()
			};
			list.addWidget(field);

			field = {
				"type": "select",
				"name": "link_slave",
				"text": "Link-slave",
				"descr": "Interface to down if link-master is down.",
				"validator": {"required": true},
				"addCurrentValue": true,
				"options": function() {
					var ifaces = [];

					/*
					 * only ethernet and E1 v16 can be link-slave,
					 * and each interface can be link-slave only one time.
					 */

					/* list of interfaces which are already link-slave */
					var linkSlaves = "";
					$.each(config.getParsed("sys_linkdeps_*"), function(num, linkdep) {
						linkSlaves += linkdep.link_slave + " ";
					});

					/* add ethernet interfaces */
					$(config.getParsed("sys_ifaces")).each(function(name, iface) {
						if (linkSlaves.search(iface) == -1 && iface.search(/eth/) != -1) {
							ifaces.push(iface);
						}
					});

					/* add E1 v16 interfaces */
					var e1Ifaces = config.getData(config.getOEM("MR16G_DRVNAME"));
					if (e1Ifaces) {
						$.each(e1Ifaces, function(num, iface) {
							if (linkSlaves.search(iface) == -1) {
								ifaces.push(iface);
							}
						});
					}

					return ifaces;
				}()
			};
			list.addWidget(field);

			list.generateList();
		}
	});

	page.generateTabs();
};

Controllers.multiplexing = function() {
	var page = this.Page();
	page.setHelpPage("multiplexing");
	page.setSubsystem("mux");

	page.addTab({
		"id": "multiplexing",
		"name": "Multiplexing",
		"func": function() {
			var colSpan = 10;
			var c = page.addContainer("multiplexing");
			c.addTitle("Multiplexing", {"colspan": colSpan});

			c.addTableHeader("DEV|MXEN|CLKM|CLKAB|CLKR|RLINE|TLINE|RFS|TFS|MXSMAP/MXRATE");

			/* describe parameters */
			c.addTableTfootStr("MXEN - enable multiplexing.", colSpan);
			c.addTableTfootStr("CLKM - clock-master or clock-slave.", colSpan);
			c.addTableTfootStr("CLKAB - clock domain (A/B).", colSpan);
			c.addTableTfootStr("CLKR - clock source (local/remote).", colSpan);
			c.addTableTfootStr("RLINE - transmit multiplexer bus line (0-15).", colSpan);
			c.addTableTfootStr("TLINE - receive multiplexer bus line (0-15).", colSpan);
			c.addTableTfootStr("RFS - receive frame start (0-255).", colSpan);
			c.addTableTfootStr("TFS - transmit frame start (0-255).", colSpan);
			c.addTableTfootStr("MXSMAP/MXRATE - multiplexing rate (for SHDSL and RS232) / " +
				"Slotmap for multiplexing (for E1).", colSpan);

			/* enables/disables CLKR field depending on CLKM value */
			var onMuxChange = function(iface) {
				if ($($.sprintf("#sys_mux_%s_clkm", iface)).val() == "0") {
					$($.sprintf("#sys_mux_%s_clkr", iface)).attr("readonly", true);
				} else {
					$($.sprintf("#sys_mux_%s_clkr", iface)).removeAttr("readonly");
				}
			};

			$.each(config.getParsed("sys_mux_ifaces"), function(num, iface) {
				var id, field;
				var row = c.addTableRow();

				field = {
					"type": "html",
					"name": iface,
					"str": iface
				};
				c.addTableWidget(field, row);

				field = {
					"type": "checkbox",
					"name": $.sprintf("sys_mux_%s_mxen", iface),
					"tip": "Enable multiplexing on this interface"
				};
				c.addTableWidget(field, row);

				id = $.sprintf("sys_mux_%s_clkm", iface);
				field = {
					"type": "select",
					"name": id,
					"id": id,
					"options": {"0": "clock-slave", "1": "clock-master"},
					"tip": "Select interface mode: <i>clock-master</i> or <i>clock-slave</i>",
					"onChange": function() {
						onMuxChange(iface);
					}
				};
				c.addTableWidget(field, row);

				field = {
					"type": "select",
					"name": $.sprintf("sys_mux_%s_clkab", iface),
					"options": {"0": "A", "1": "B"},
					"tip": "Select interface clock domain: <i>A</i> or <i>B</i>"
				};
				c.addTableWidget(field, row);

				id = $.sprintf("sys_mux_%s_clkr", iface);
				field = {
					"type": "select",
					"name": id,
					"id": id,
					"options": {"0": "local", "1": "remote"},
					"tip": "Select clock source: <i>remote</i> or <i>local</i> (for <i>clock-master</i> interface only)"
				};
				c.addTableWidget(field, row);

				field = {
					"type": "text",
					"name": $.sprintf("sys_mux_%s_rline", iface),
					"defaultValue": "0",
					"tip": "Enter rline number (<i>0-15</i>)",
					"validator": {"required": true, "min": 0, "max": 15}
				};
				c.addTableWidget(field, row);

				field = {
					"type": "text",
					"name": $.sprintf("sys_mux_%s_tline", iface),
					"defaultValue": "0",
					"tip": "Enter tline number (<i>0-15</i>)",
					"validator": {"required": true, "min": 0, "max": 15}
				};
				c.addTableWidget(field, row);

				field = {
					"type": "text",
					"name": $.sprintf("sys_mux_%s_rfs", iface),
					"defaultValue": "0",
					"tip": "Enter recieve frame start number (<i>0-255</i>)",
					"validator": {"required": true, "min": 0, "max": 255}
				};
				c.addTableWidget(field, row);

				field = {
					"type": "text",
					"name": $.sprintf("sys_mux_%s_tfs", iface),
					"defaultValue": "0",
					"tip": "Enter transmit frame start number (<i>0-255</i>)",
					"validator": {"required": true, "min": 0, "max": 255}
				};
				c.addTableWidget(field, row);

				var rate, tip, validator, defaultValue;
				if (iface.search("E1") != -1) {
					rate = "mxsmap";
					tip = "Enter <i>mxsmap</i> for E1 interface. <i>mxsmap</i> is a map of time-slots (e.g., <i>1-31</i>). This value can be changed after saving.";
					validator = {"smap": true};
					defaultValue = "";
				} else {
					rate = "mxrate";
					tip = "Enter <i>mxrate</i> for DSL interface. <i>mxrate</i> is a number of time-slots (e.g., <i>12</i>).";
					validator = {"required": true, "min": 0};
					defaultValue = "0";
				}

				id = $.sprintf("sys_mux_%s_%s", iface, rate);
				field = {
					"type": "text",
					"name": id,
					"id": id,
					"tip": tip,
					"defaultValue": defaultValue,
					"validator": validator
				};
				c.addTableWidget(field, row);

				onMuxChange(iface);
			});

			c.addSubmit({
				"onSubmit": function() {
					/* remove previous output */
					$(".cmdOutput").remove();
				},
				"onSuccess": function() {
					/* MXSMAP can be cnanged by system */
					var fields = new Array();
					$.each(config.getParsed("sys_mux_ifaces"), function(num, iface) {
						if (iface.search("E1") != -1) {
							fields.push($.sprintf("sys_mux_%s_mxsmap", iface));
						}
					});
					updateFields(fields);

					/* execute command */
					c.addConsoleToForm("/sbin/mxconfig --check");
				}
			});

			c.addConsoleToForm("/sbin/mxconfig --check");
		}
	});

	page.generateTabs();
};

function mr17sModuleName(pcislot) {
	/* get type (DTE or DCE) for this RS232 node */
	var rs232Type = config.getCachedOutput($.sprintf("rs232Type_%s", pcislot));
	if (rs232Type != "undefined") {
		rs232Type = "-" + rs232Type;
	} else {
		rs232Type = "";
	}

	return $.sprintf("%s%s%s%s", config.getOEM("MR17S_MODNAME"), config.getOEM("OEM_IFPFX"),
			config.get($.sprintf("sys_pcitbl_s%s_ifnum", pcislot)), rs232Type);
}

Controllers.rs232 = function(node, pcislot, pcidev) {
	var page = this.Page();
	page.setSubsystem($.sprintf("rs232.%s.%s", pcislot, pcidev));

	/* settings tab */
	page.addTab({
		"id": "settings",
		"name": "RS232 settings",
		"func": function() {
			var c, field;
			c = page.addContainer("settings");

			c.addTitle($.sprintf("%s (module %s, slot %s) settings", node,
					mr17sModuleName(pcislot), pcislot - 2));

			field = {
				"type": "checkbox",
				"name": $.sprintf("sys_mux_%s_mxen", node),
				"text": "Enable multiplexing",
				"descr": "Enable multiplexing on this interface",
				"tip": "This option is equivalent to MXEN on a multiplexing page."
			};
			c.addWidget(field);

			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_baudrate", pcislot, pcidev),
				"text": "Baud rate",
				"options": [230400,115200,57600,38400,28800,19200,14400,9600,7200,4800,3600,2400,1800,1200,600,300]
			};
			c.addWidget(field);

			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_cs", pcislot, pcidev),
				"text": "Character size (bits)",
				"options": {"cs7": "7", "cs8": "8"}
			};
			c.addWidget(field);

			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_stopb", pcislot, pcidev),
				"text": "Stop bits",
				"options": {"-cstopb": "1", "cstopb": "2"}
			};
			c.addWidget(field);

			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_parity", pcislot, pcidev),
				"text": "Parity",
				"options": "none even odd"
			};
			c.addWidget(field);

			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_fctrl", pcislot, pcidev),
				"text": "Hardware Flow control",
				"options": {"0": "off", "1": "on"}
			};
			c.addWidget(field);

			field = {
				"type": "select",
				"name": $.sprintf("sys_pcicfg_s%s_%s_sigfwd", pcislot, pcidev),
				"text": "Forward Modem Signals",
				"options": {"0": "off", "1": "on"}
			};
			c.addWidget(field);

			c.addSubmit();
		}
	});

	page.generateTabs();
};

Controllers.adm5120sw = function() {
	var page = this.Page();
	page.setHelpPage("adm5120sw");

	page.addTab({
		"id": "adm5120sw",
		"name": "Internal switch configuration",
		"func": function() {
			var c, field;
			c = page.addContainer("adm5120sw");
			c.addTitle("Internal switch configuration");

			/* make list of ethernet interfaces */
			var ethIfaces = new Object();
			$.each(config.getParsed("sys_switch_ports"), function(num, port) {
				ethIfaces[port] = "eth" + port;
			});

			c.addTableHeader("Port|Interface");
			c.addTableTfootStr("Device has to be rebooted to apply changes.", 2);
			$.each(config.getParsed("sys_switch_ports"), function(num, port) {
				var row = c.addTableRow();

				field = {
					"type": "html",
					"name": port,
					"str": "Port " + port
				};
				c.addTableWidget(field, row);

				field = {
					"type": "select",
					"name": $.sprintf("sys_switch_port%s_iface", port),
					"tip": "Attach port to selected interface",
					"options": ethIfaces
				};
				c.addTableWidget(field, row);
			});

			c.setSuccessMessage("Device has to be rebooted to apply changes.");
			c.addSubmit();
		}
	});

	page.generateTabs();
};
