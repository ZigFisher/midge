/*
 * Renders content of network General tab.
 */
Controllers.ifaceGeneral = function(c, iface) {
	var field;
	c.setSubsystem("network." + iface);
	c.setHelpPage("iface");
	c.setHelpSection("general");
	c.addTitle("Interface general settings");

	var showMethod = function() {
		/* remove method's widgets */
		$(".tmpMethod").parents("tr").remove();

		if ($("#method").val() == "static") {
			if (config.get($.sprintf("sys_iface_%s_proto", iface)) == "hdlc") {
				field = {
					"type": "text",
					"name": $.sprintf("sys_iface_%s_pointopoint_local", iface),
					"text": "Point to Point local",
					"descr": "Point-to-Point local address.",
					"tip": "e.g., 10.0.0.1",
					"validator": {"required": true, "ipAddr": true},
					"cssClass": "tmpMethod"
				};
				c.addWidget(field);

				field = {
					"type": "text",
					"name": $.sprintf("sys_iface_%s_pointopoint_remote", iface),
					"text": "Point to Point remote",
					"descr": "Point-to-Point remote address.",
					"tip": "e.g., 10.0.0.2",
					"validator": {"required": true, "ipAddr": true},
					"cssClass": "tmpMethod"
				};
				c.addWidget(field);
			} else {
				field = {
					"type": "text",
					"name": $.sprintf("sys_iface_%s_ipaddr", iface),
					"id": "ipaddr",
					"text": "Static address",
					"descr": "IP address.",
					"tip": "e.g., 192.168.2.100",
					"validator": {"required": true, "ipAddr": true},
					"cssClass": "tmpMethod"
				};
				c.addWidget(field);

				field = {
					"type": "text",
					"name": $.sprintf("sys_iface_%s_netmask", iface),
					"text": "Netmask",
					"descr": "Network mask.",
					"tip": "e.g., 255.255.255.0",
					"validator": {"required": true, "netmask": true},
					"cssClass": "tmpMethod"
				};
				c.addWidget(field);

				field = {
					"type": "text",
					"name": $.sprintf("sys_iface_%s_broadcast", iface),
					"text": "Broadcast",
					"descr": "Broadcast address.",
					"tip": "e.g., 192.168.2.255",
					"validator": {"ipAddr": true},
					"cssClass": "tmpMethod"
				};
				c.addWidget(field);

				field = {
					"type": "text",
					"name": $.sprintf("sys_iface_%s_gateway", iface),
					"text": "Gateway",
					"descr": "Default gateway.",
					"tip": "e.g., 192.168.2.1",
					"validator": {"ipAddr": true},
					"cssClass": "tmpMethod"
				};
				c.addWidget(field);
			}
		}
	};

	field = {
		"type": "text",
		"name": $.sprintf("sys_iface_%s_desc", iface),
		"text": "Description",
		"descr": "Description of interface."
	};
	c.addWidget(field);

	/* this interface is bridge */
	if (iface.search("br") != -1) {
		field = {
			"type": "text",
			"name": $.sprintf("sys_iface_%s_br_ifaces", iface),
			"id": "ifaces",
			"text": "Bridge interfaces",
			"descr": "Interfaces for bridge separated by space.",
			"tip": "<b>Example:</b> eth0 eth1 dsl0<br><b>Note:</b> You can use only" +
					" Ethernet-like interfaces, like ethX, dslX<br><b>Note:</b> Interfaces should" +
					" be enabled, but <b>auto</b> should be switched <b>off</b>.",
			"validator": {"required": true}
		};
		c.addWidget(field);
	/* this interface is bonding */
	} else if (iface.search("bond") != -1) {
		field = {
			"type": "text",
			"name": $.sprintf("sys_iface_%s_bond_ifaces", iface),
			"id": "ifaces",
			"text": "Bonding interfaces",
			"descr": "Interfaces for bonding separated by space.",
			"tip": "<b>Example:</b>eth0 eth1 dsl0<br><b>Note:</b>You can use only Ethernet-like" +
					" interfaces, like ethX, dslX, bondX<br><b>Note:</b> Interfaces should be" +
					" enabled, but <b>auto</b> should be switched <b>off</b>.",
			"validator": {"required": true}
		};
		c.addWidget(field);
	}

	field = {
		"type": "checkbox",
		"name": $.sprintf("sys_iface_%s_enabled", iface),
		"id": "enabled",
		"text": "Enabled",
		"descr": "If set, interface can be start on boot or by another interface."
	};
	c.addWidget(field);

	field = {
		"type": "checkbox",
		"name": $.sprintf("sys_iface_%s_auto", iface),
		"text": "Auto",
		"descr": "If set and interface is enabled, it will be start on boot."
	};
	c.addWidget(field);

	if (config.get($.sprintf("sys_iface_%s_proto", iface)) != "vlan") {
		var dependList = new Object();
		$.each(config.getParsed("sys_ifaces"), function(name, value) {
			dependList[value] = value;
		});
		dependList.none = "None";
		field = {
			"type": "select",
			"name": "sys_iface_" + iface + "_depend_on",
			"id": "depend_on",
			"text": "Depends on",
			"options": dependList,
			"defaultValue": "none",
			"descr": $.sprintf("Start specified interface before this (%s) interface.", iface)
		};
		c.addWidget(field);
	}

	field = {
		"type": "select",
		"name": $.sprintf("sys_iface_%s_method", iface),
		"id": "method",
		"text": "Method",
		"descr": "Method of setting IP address.",
		"options": config.get($.sprintf("sys_iface_%s_proto", iface)) == "hdlc" ?
			{"none": "None", "static": "Static address"} :
			{"none": "None", "static": "Static address", "zeroconf": "Zero Configuration",
					"dynamic": "Dynamic address"},
		"onChange": showMethod
	};
	c.addWidget(field);

	/* TODO
	field= {
		"type": "checkbox",
		"name": "sys_iface_" + iface + "_opt_accept_redirects",
		"text": "Accept redirects"
	};
	c.addWidget(field);

	field = {
		"type": "checkbox",
		"name": "sys_iface_" + iface + "_opt_forwarding",
		"text": "Forwarding"
	};
	c.addWidget(field);

	field = {
		"type": "checkbox",
		"name": "sys_iface_" + iface + "_opt_proxy_arp",
		"text": "Proxy ARP"
	};
	c.addWidget(field);

	field = {
		"type": "checkbox",
		"name": "sys_iface_" + iface + "_opt_rp_filter",
		"text": "RP Filter"
	};
	c.addWidget(field);
	*/

	showMethod();

	/* check that web interface will be available via network interfaces and set parameters for slaves */
	var additionalKeys = [];
	c.addSubmit({
		"additionalKeys": additionalKeys,
		"preSubmit": function() {
			/* remove old keys from additionalKeys */
			additionalKeys.splice(0, additionalKeys.length);
			
			/* if this interface is used for accessing the web interface */
			if (window.location.hostname.search(
					config.get($.sprintf("sys_iface_%s_ipaddr", iface))) != -1) {
				/* if this interface have static IP address */
				if ($("#ipaddr").length > 0) {
					/* if IP address was changed */
					if (config.get($.sprintf("sys_iface_%s_ipaddr", iface)) != $("#ipaddr").val()) {
						c.setSuccessMessage(_("IP address for this interface is changed. This page will be closed in 5 seconds. Web interface will be available at") + " " + $("#ipaddr").val());
						setTimeout(function() {
							document.write(
									$.sprintf("<html><head><title>%s</title></head><body><h2>%s</h2><br><a href='https://" + $("#ipaddr").val() + "/wf2/'>%s</a></body></html>",
									_("IP address is changed"),
									_("IP address of network interface which was used for accessing the web interface was changed, use new IP address") + " " + "(" + $("#ipaddr").val() + ").",
									_("Reload web-interface from new IP address")));
							document.close();
						}, 5000);
					}
				/* do not save form - this interface must have static IP address */
				} else {
					c.setError("This network interface must have static IP address to allow you to access the web interface.");
					c.showMsg();
					return false;
				}
			}

			/* if #ifaces widget is exist - bridge/bonding is setuped */
			if ($("#ifaces").length > 0) {
				var saveAllowed = true;
				var errInterface;
				$.each($("#ifaces").val().split(" "), function(num, slaveIface) {
					/* if slave interface is used for accessing the webface */
					if (window.location.hostname.search(
							config.get($.sprintf("sys_iface_%s_ipaddr", slaveIface))) != -1) {
						/* bridge/bonding must have static ipaddr */
						if ($("#ipaddr").length == 0) {
							saveAllowed = false;
							errInterface = slaveIface;
							return false;
						}
					}
					
					/* if this interface is enabled */
					if ($("#enabled").attr("checked") == true) {
						/* set auto=0 enabled=1 method=none for slave interfaces */
						$.addObjectWithProperty(additionalKeys,
								$.sprintf("sys_iface_%s_auto", slaveIface), "0");
						$.addObjectWithProperty(additionalKeys,
								$.sprintf("sys_iface_%s_enabled", slaveIface), "1");
						$.addObjectWithProperty(additionalKeys,
								$.sprintf("sys_iface_%s_method", slaveIface), "none");
					}
				});

				if (!saveAllowed) {
					c.setError(_("You are including network interface")
							+ $.sprintf(" (%s), ", errInterface)
							+ _("which is used for accessing the web interface, so current network interface")
							+ $.sprintf(" (%s) ", iface)
							+ _("must have static IP address to allow you to access the web interface."));
					c.showMsg();
					return false;
				}
			}
		}
	});
};

Controllers.dynamic_ifaces = function() {
	var page = this.Page();
	page.setHelpPage("ifaces");

	page.addTab({
		"id": "dynamic_ifaces",
		"name": "Dynamic ifaces",
		"func": function() {
			/* update list of ifaces for deletion */
			var setIfaces = function() {
				var ifaces = new Array();
				$.each(config.getParsed("sys_ifaces"), function(name, value) {
					if (value.search(/\w+\d+v\d+/) != -1 || value.search(/eth|dsl|E1/) == -1) {
						ifaces.push(value);
					}
				});

				$("#del_iface").setOptionsForSelect({"options": ifaces});
			};


			var c, field;
			c = page.addContainer("dynamic_ifaces");
			c.addTitle("Add dynamic network interface");

			/* IDs of VLAN widgets */
			var vlanWidgetsIDs = new Array();

			/* add, if not exist, widgets for VLAN */
			var addVlanWidgets = function() {
				if (vlanWidgetsIDs.length != 0) return;

				/* phys_iface */
				var physIfaces = new Array();
				$.each(config.getParsed("sys_ifaces"), function(name, value) {
					if (value.search(/\w+\d+v\d+/) == -1) {
						physIfaces.push(value);
					}
				});

				vlanWidgetsIDs.push("phys_iface");
				field = {
					"type": "select",
					"name": "phys_iface",
					"text": "Physical interface",
					"options": physIfaces
				};
				c.addWidget(field);

				/* vlan_id */
				vlanWidgetsIDs.push("vlan_id");
				field = {
					"type": "text",
					"name": "vlan_id",
					"text": "VLAN ID"
				};
				c.addWidget(field);
			};

			/* remove, if exist, widgets for VLAN */
			var removeVlanWidgets = function() {
				if (vlanWidgetsIDs.length != 0) {
					$.each(vlanWidgetsIDs, function(num, value) {
						$("#" + value).parents("tr").remove();
					});
					vlanWidgetsIDs = new Array();
				}
			};

			field = {
				"type": "select",
				"name": "iface_proto",
				"id": "iface_proto",
				"text": "Protocol",
				"descr": "Please select interface protocol",
				"options": {"bridge": "Bridge", "bonding": "Bonding", "vlan": "VLAN"},
				"onChange": function() {
					if ($("#iface_proto").val() == "vlan") {
						addVlanWidgets();
					} else {
						removeVlanWidgets();
					}
				}
			};
			c.addWidget(field);

			c.addSubmit({
				"submitName": "Add",
				"noSubmit": true,
				"onSubmit": function() {
					var parameters = {"proto": $("#iface_proto").val()};

					/* set parameters for VLAN interface */
					if ($("#iface_proto").val() == "vlan") {
						/* set real (in system) interface name */
						parameters.real =
							$.sprintf("%s.%s", $("#phys_iface").val(), $("#vlan_id").val());

						/* set interface name, used in KDB (and webface) */
						parameters.iface =
							$.sprintf("%sv%s", $("#phys_iface").val(), $("#vlan_id").val());

						/* set dependOn interface and VLAN ID */
						parameters.dependOn = $("#phys_iface").val();
						parameters.vlanId = $("#vlan_id").val();
					}
					var newIface = config.addIface(parameters);

					setIfaces();

                    if ($("#iface_proto").val() == "bridge" || $("#iface_proto").val() == "bonding") {
                        Controllers.fastBridgeOrBondSetup(newIface);
                    }
				}
			});

			page.addBr("dynamic_ifaces");
			var c2 = page.addContainer("dynamic_ifaces");
			c2.addTitle("Delete dynamic network interface");

			field = {
				"type": "select",
				"name": "del_iface",
				"id": "del_iface",
				"text": "Interface",
				"descr": "Interface to delete",
				"options": ""
			};
			c2.addWidget(field);

			setIfaces();

			c2.addSubmit({
				"submitName": "Delete",
				"noSubmit": true,
				"onSubmit": function() {
					if ($("#del_iface").val() != null) {
						config.delIface($("#del_iface").val());
						setIfaces();
                        return true;
					} else {
                        return false;
                    }
				}
			});
		}
	});

	page.generateTabs();
};

/*
 * Controller for fast & simple bridge/bonding setup.
 */
Controllers.fastBridgeOrBondSetup = function(iface) {
    var page = this.Page();

	page.addTab({
		"id": "fast_setup",
		"name": $.sprintf("Fast %s configuration (%s)",
				(iface.search("br") != -1) ? "bridge" : "bonding", iface),
		"func": function() {
			var c = page.addContainer("fast_setup");
			Controllers.ifaceGeneral(c, iface);
		}
	});

	page.generateTabs();
};

Controllers.fw = function() {
	var page = this.Page();
	page.setSubsystem("fw");
	page.setHelpPage("fw");

	/* settings tab */
	page.addTab({
		"id": "settings",
		"name": "Firewall settings",
		"func": function() {
			var c, field;
			c = page.addContainer("settings");
			c.addTitle("Firewall settings");

			field = {
				"type": "checkbox",
				"name": "sys_fw_enabled",
				"text": "Enable firewall",
				"descr": "Firewall allows you to perform packet filtering."
			};
			c.addWidget(field);

			c.addSubmit();
		}
	});

	/*
	 * Event handler.
	 * When adding or editing FW rule:
	 * when selected targets DNAT or SNAT, add NatTo widget.
	 *
	 * list — current list to add new widget to (passed automatically by framework).
	 */
	var onChangeTargetOrProto = function(list) {
		var field;

		/* get type of target */
		var target = $("#target").val();

		/* if target is DNAT or SNAT */
		if (target == "DNAT" || target == "SNAT") {
			/* if there is no NatTo field — add it */
			if ($("#natto").length == 0) {
				/* add new field */
				field = {
					"type": "text",
					"name": "natto",
					"text": "Nat to address",
					"descr": target == "DNAT" ? "Do Destination NAT to address."
											  : "Do Source NAT to address.",
					"validator": {"required": true, "ipAddrPort": true},
					"tip": "You can add port number after IP address<br><i>Example: 192.168.0.1:80</i>."
				};
				list.addDynamicWidget(field,
					{"type": "insertAfter", "anchor": $("#dst").parents("tr")});
			}
		/* remove field */
		} else $("#natto").parents("tr").remove();

		/* get proto */
		var proto = $("#proto").val();

		/* if protocol is TCP or UDP */
		if (proto == "tcp" || proto == "udp") {
			/* if there is no dport field (and in this case sport too) — add it */
			if ($("#dport").length == 0) {
				tip = "An inclusive range can also be specified, using the format <b>port:port</b>. " +
						"If the first port is omitted, 0 is assumed; if the last is omitted, 65535 is assumed.";

				field = {
					"type": "text",
					"name": "sport",
					"text": "Source port",
					"descr": "Source port or port range.",
					"validator": {"required": true, "ipPortRange": true},
					"defaultValue": "any",
					"tip": tip
				};
				list.addDynamicWidget(field,
					{"type": "insertAfter", "anchor": $("#proto").parents("tr")});

				field = {
					"type": "text",
					"name": "dport",
					"text": "Destination port",
					"descr": "Destination port or port range.",
					"validator": {"required": true, "ipPortRange": true},
					"defaultValue": "any",
					"tip": tip
				};
				list.addDynamicWidget(field,
					{"type": "insertAfter", "anchor": $("#proto").parents("tr")});
			}
		/* remove fields */
		} else {
			$("#sport").parents("tr").remove();
			$("#dport").parents("tr").remove();
		}
	};

	/*
	 * Adds widgets for adding or editing FW rules.
	 *
	 * options — array of options:
	 *  - list — list object to add widgets to;
	 *  - chain — name of chain for which widgets is added.
	 */
	var addFwAddingRuleWidgets = function(options) {
		var field;

		field = {
			"type": "checkbox",
			"name": "enabled",
			"text": "Enabled",
			"descr": "Enable rule."
		};
		options.list.addWidget(field);

		field = {
			"type": "text",
			"name": "name",
			"text": "Short name",
			"descr": "Name of rule.",
			"validator": {"required": true, "alphanumU": true}
		};
		options.list.addWidget(field);

		field = {
			"type": "select",
			"name": "proto",
			"text": "Protocol",
			"descr": "Protocol of packet.",
			"defaultValue": "all",
			"options": "all tcp udp icmp",
			"onChange": onChangeTargetOrProto
		};
		options.list.addWidget(field);

		var tip = "IP address can be either a network IP address (with /mask), or a plain IP address. " +
				"A ! argument before the address specification inverts the sense of the address." +
				"<br><b>Examples:</b> 192.168.1.0/24, 192.168.1.5<br> Use 0.0.0.0/0 for <b>any</b>.";

		field = {
			"type": "text",
			"name": "src",
			"text": "Source IP",
			"descr": "Source address",
			"validator": {"required": true, "ipNetMaskIptables": true},
			"defaultValue": "0.0.0.0/0",
			"tip": tip
		};
		options['list'].addWidget(field);

		field = {
			"type": "text",
			"name": "dst",
			"text": "Destination IP",
			"descr": "Destination address",
			"validator": {"required": true, "ipNetMaskIptables": true},
			"defaultValue": "0.0.0.0/0",
			"tip": tip
		};
		options['list'].addWidget(field);

		var targets = "ACCEPT DROP";

		/* depending on chain add additional targets */
		switch (options['chain']) {
			case "PREROUTING":
				targets += " DNAT";
				break;

			case "POSTROUTING":
				targets += " SNAT MASQUERADE";
				break;

			case "INPUT":
			case "OUTPUT":
			case "FORWARD":
				targets += " REJECT";
				break;
		}

		field = {
			"type": "select",
			"name": "target",
			"text": "Action",
			"descr": "What to do with packet",
			"defaultValue": "ACCEPT",
			"options": targets,
			"onChange": onChangeTargetOrProto
		};
		options['list'].addWidget(field);
	};

	/* FILTER tab */
	page.addTab({
		"id": "filter",
		"name": "Filter",
		"func": function() {
			var c, field, list;

			/* policies */
			c = page.addContainer("filter");
			c.setHelpSection("filter_policy");
			c.addTitle("Default policies");

			field = {
				"type": "select",
				"name": "sys_fw_filter_policy_forward",
				"text": "Default policy for FORWARD",
				"options": "ACCEPT DROP"
			};
			c.addWidget(field);

			field = {
				"type": "select",
				"name": "sys_fw_filter_policy_input",
				"text": "Default policy for INPUT",
				"options": "ACCEPT DROP"
			};
			c.addWidget(field);

			field = {
				"type": "select",
				"name": "sys_fw_filter_policy_output",
				"text": "Default policy for OUTPUT",
				"options": "ACCEPT DROP"
			};
			c.addWidget(field);

			c.addSubmit();

			/* FORWARD chain */
			page.addBr("filter");
			c = page.addContainer("filter");
			c.setHelpSection("filter_forward");

			/* create list of forward rules */
			list = c.createList({
				"tabId": "filter",
				"header": ["Name", "Src", "Dst", "Proto", "Src port", "Dst port", "Action"],
				"varList": ["name", "src", "dst", "proto", "sport", "dport", "target"],
				"listItem": "sys_fw_filter_forward_",
				"onAddOrEditItemRender": onChangeTargetOrProto,
				"addMessage": _("Add rule to FORWARD chain"),
				"editMessage": _("Edit rule in FORWARD chain"),
				"listTitle": _("Filter, FORWARD chain"),
				"helpPage": "filter",
				"helpSection": "filter_add"
			});

			addFwAddingRuleWidgets({
				"list": list,
				"chain": "FORWARD"
			});

			list.generateList();

			/* INPUT chain */
			page.addBr("filter");
			c = page.addContainer("filter");
			c.setHelpSection("filter_input");

			/* create list of input rules */
			list = c.createList({
				"tabId": "filter",
				"header": ["Name", "Src", "Dst", "Proto", "Src port", "Dst port", "Action"],
				"varList": ["name", "src", "dst", "proto", "sport", "dport", "target"],
				"listItem": "sys_fw_filter_input_",
				"onAddOrEditItemRender": onChangeTargetOrProto,
				"addMessage": _("Add rule to INPUT chain"),
				"editMessage": _("Edit rule in INPUT chain"),
				"listTitle": _("Filter, INPUT chain"),
				"helpPage": "filter",
				"helpSection": "filter_add"
			});

			addFwAddingRuleWidgets({
				"list": list,
				"chain": "INPUT"
			});

			list.generateList();

			/* OUTPUT chain */
			page.addBr("filter");
			c = page.addContainer("filter");
			c.setHelpSection("filter_output");

			/* create list of output rules */
			list = c.createList({
				"tabId": "filter",
				"header": ["Name", "Src", "Dst", "Proto", "Src port", "Dst port", "Action"],
				"varList": ["name", "src", "dst", "proto", "sport", "dport", "target"],
				"listItem": "sys_fw_filter_output_",
				"onAddOrEditItemRender": onChangeTargetOrProto,
				"addMessage": _("Add rule to OUTPUT chain"),
				"editMessage": _("Edit rule in OUTPUT chain"),
				"listTitle": _("Filter, OUTPUT chain"),
				"helpPage": "filter",
				"helpSection": "filter_add"
			});

			addFwAddingRuleWidgets({
				"list": list,
				"chain": "OUTPUT"
			});

			list.generateList();
		}
	});

	/* NAT tab */
	page.addTab({
		"id": "nat",
		"name": "NAT",
		"func": function() {
			var c, field, list;

			/* policies */
			c = page.addContainer("nat");
			c.setHelpPage("nat");
			c.setHelpSection("nat_policy");
			c.addTitle("Default policies");

			field = {
				"type": "select",
				"name": "sys_fw_nat_policy_prerouting",
				"text": "Default policy for PREROUTING",
				"options": "ACCEPT DROP"
			};
			c.addWidget(field);

			field = {
				"type": "select",
				"name": "sys_fw_nat_policy_postrouting",
				"text": "Default policy for POSTROUTING",
				"options": "ACCEPT DROP"
			};
			c.addWidget(field);

			c.addSubmit();

			/* PREROUTING chain */
			page.addBr("nat");
			c = page.addContainer("nat");
			c.setHelpPage("nat");
			c.setHelpSection("nat_prerouting");

			/* create list of prerouting rules */
			list = c.createList({
				"tabId": "nat",
				"header": ["Name", "Src", "Dst", "Proto", "Src port", "Dst port", "Action"],
				"varList": ["name", "src", "dst", "proto", "sport", "dport", "target"],
				"listItem": "sys_fw_nat_prerouting_",
				"onAddOrEditItemRender": onChangeTargetOrProto,
				"addMessage": _("Add rule to PREROUTING chain"),
				"editMessage": _("Edit rule in PREROUTING chain"),
				"listTitle": _("NAT, PREROUTING chain"),
				"helpPage": "nat",
				"helpSection": "nat_add"
			});

			addFwAddingRuleWidgets({
				"list": list,
				"chain": "PREROUTING"
			});

			list.generateList();

			/* POSTROUTING chain */
			page.addBr("nat");
			c = page.addContainer("nat");
			c.setHelpPage("nat");
			c.setHelpSection("nat_postrouting");

			/* create list of postrouting rules */
			list = c.createList({
				"tabId": "nat",
				"header": ["Name", "Src", "Dst", "Proto", "Src port", "Dst port", "Action"],
				"varList": ["name", "src", "dst", "proto", "sport", "dport", "target"],
				"listItem": "sys_fw_nat_postrouting_",
				"onAddOrEditItemRender": onChangeTargetOrProto,
				"addMessage": _("Add rule to POSTROUTING chain"),
				"editMessage": _("Edit rule in POSTROUTING chain"),
				"listTitle": _("NAT, POSTROUTING chain"),
				"helpPage": "nat",
				"helpSection": "nat_add"
			});

			addFwAddingRuleWidgets({
				"list": list,
				"chain": "POSTROUTING"
			});

			list.generateList();
		}
	});

	page.generateTabs();
};

Controllers.iface = function(iface) {
	var page = this.Page();
	page.setSubsystem("network." + iface);
	page.setHelpPage("iface");

	/* set info message about master interfaces or other information */
	var setInfoText = function(c) {
		/* find name of master interfaces, if it exists */
		var master = "";
		var mIfaces = config.getByRegexp(/(sys_iface_)*((_br_ifaces)|(_bond_ifaces))/);
		$.each(mIfaces, function(mIfaceKey, mIface) {
			if (mIface.search(iface) != -1) {
				master += mIfaceKey.replace("sys_iface_", "").replace("_br_ifaces", "")
						.replace("_bond_ifaces", "") + " ";
			}
		});
		master = $.trim(master);

		/* check if this interface is used for accessing the web interface */
		var control = false;
		if (window.location.hostname.search(
					config.get($.sprintf("sys_iface_%s_ipaddr", iface))) != -1) {
			control = true;
		}

		var msg = "";
		if (master.length > 0) {
			msg += _("This interface is a part of") + " " + master + ".";
		}
		if (control) {
			if (msg.length > 0) {
				msg += "<br>";
			}
			msg += _("This network interface is used for accessing the web interface.");
		}

		if (msg.length > 0) {
			c.addStaticMessage(msg);
		}
	};

	/* STATUS tab */
	page.addTab({
		"id": "status",
		"name": "Status",
		"func": function() {
			var c, field;
			var realIface = config.get($.sprintf("sys_iface_%s_real", iface)) ?
				config.get($.sprintf("sys_iface_%s_real", iface)) : iface;

			c = page.addContainer("status");
			c.addTitle("Interface status");
			setInfoText(c);

			/* add general widget, which will contain three buttons */
			field = {
				"type": "general",
				"name": "interface_ctrl",
				"text": "Interface control"
			};
			c.addWidget(field);

			/* restart button */
			field = {
				"type": "button",
				"name": "btn_restart",
				"text": "Restart",
				"func": function(thisContainer, src) {
					$(src.currentTarget).attr("disabled", true);
					config.cmdExecute({
						"cmd": $.sprintf("/etc/init.d/network restart %s", realIface),
						"callback": function() {
							c.containerRedraw();
						}
					});
				}
			};
			c.addSubWidget(field, {
					"type": "prependToAnchor",
					"anchor": "#td_interface_ctrl"
				}
			);

			/* stop button */
			field = {
				"type": "button",
				"name": "btn_stop",
				"text": "Stop",
				"func": function(thisContainer, src) {
					$(src.currentTarget).attr("disabled", true);
					config.cmdExecute({
						"cmd": $.sprintf("/sbin/ifdown %s", realIface),
						"callback": function() {
							$(src.currentTarget).removeAttr("disabled");
							c.containerRedraw();
						}
					});
				}
			};
			c.addSubWidget(field, {
					"type": "prependToAnchor",
					"anchor": "#td_interface_ctrl"
				}
			);

			/* start button */
			field = {
				"type": "button",
				"name": "btn_start",
				"text": "Start",
				"func": function(thisContainer, src) {
					$(src.currentTarget).attr("disabled", true);
					config.cmdExecute({
						"cmd": $.sprintf("/sbin/ifup %s", realIface),
						"callback": function() {
							$(src.currentTarget).removeAttr("disabled");
							c.containerRedraw();
						}
					});
				}
			};
			c.addSubWidget(field, {
					"type": "prependToAnchor",
					"anchor": "#td_interface_ctrl"
				}
			);

			page.addBr("status");
			c = page.addContainer("status");
			c.addTitle("Interface status");
			c.addConsole(["/sbin/ifconfig " + realIface, "/usr/sbin/ip addr show dev " + realIface,
				"/usr/sbin/ip link show dev " + realIface]);

			page.addBr("status");
			c = page.addContainer("status");
			c.addTitle("Routes");
			c.addConsole("/usr/sbin/ip route show dev " + realIface);

			page.addBr("status");
			c = page.addContainer("status");
			c.addTitle("ARP");
			c.addConsole("/usr/sbin/ip neigh show dev " + realIface);

			/* add additional info for bridge interface */
			if (realIface.search(/^br/) != -1) {
				page.addBr("status");
				c = page.addContainer("status");
				c.addTitle("System bridges");
				c.addConsole("/usr/sbin/brctl show");

				page.addBr("status");
				c = page.addContainer("status");
				c.addTitle($.sprintf("Bridge %s info", realIface));
				c.addConsole("/usr/sbin/brctl showmacs " + realIface);

				page.addBr("status");
				c = page.addContainer("status");
				c.addTitle($.sprintf("STP bridge %s info", realIface));
				c.addConsole("/usr/sbin/brctl showstp " + realIface);
			}

			/* show switch info for ethernet interfaces */
			if (realIface.search(/^eth/) != -1) {
				page.addBr("status");
				c = page.addContainer("status");
				c.addTitle("Internal ethernet switch status");
				c.addConsole("/bin/cat /proc/sys/net/adm5120sw/status");
			}
		}
	});

	/* GENERAL tab */
	page.addTab({
		"id": "general",
		"name": "General",
		"func": function() {
			var c = page.addContainer("general");
			setInfoText(c);
			Controllers.ifaceGeneral(c, iface);
		}
	});

	/* SPECIFIC tab */
	page.addTab({
		"id": "specific",
		"name": "Specific",
		"func": function() {
			var c, field;
			c = page.addContainer("specific");
			setInfoText(c);

			switch (config.get("sys_iface_" + iface + "_proto"))
			{
				case "ether":
					c.addTitle("Ethernet Specific parameters");

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_mac",
						"text": "MAC address",
						"descr": "MAC address for the interface",
						"tip": "e.g., 00:ff:1f:00:75:99",
						"validator": {"macAddr": true}
					};
					c.addWidget(field);

					c.addSubmit();

					break;

				case "pppoe":
					c.addTitle("PPPoE Specific parameters");

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_pppoe_iface",
						"text": "Interface",
						"descr": "Parent interface name",
						"validator": {"required": true}
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_pppoe_service",
						"text": "Service",
						"descr": "Desired service name",
						"tip": "Router will only initiate sessions with access concentrators which" +
							" can provide the specified service.<br>  In most cases, you should <b>not</b>" +
							" specify this option."
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_pppoe_ac",
						"text": "Access Concentrator",
						"descr": "Desired access concentrator name",
						"tip": "Router will only initiate sessions with the specified access concentrator." +
							" In most cases, you should <b>not</b> specify this option. Use it only if you" +
							" know that there are multiple access concentrators."
					};
					c.addWidget(field);

					field = {
						"type": "checkbox",
						"name": "sys_iface_" + iface + "_pppoe_defaultroute",
						"text": "Default route",
						"descr": "Add a default route to the system routing tables, using the peer as the gateway"
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_pppoe_username",
						"text": "Username",
						"validator": {"required": true}
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_pppoe_password",
						"text": "Password",
						"validator": {"required": true}
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_pppoe_pppdopt",
						"text": "PPPD options",
						"defaultValue": "noauth nobsdcomp nodeflate"
					};
					c.addWidget(field);

					c.addSubmit();

					break;

				case "pptp":
					c.addTitle("PPtP Specific parameters");

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_pptp_server",
						"text": "Server",
						"descr": "PPtP server",
						"validator": {"required": true, "domainNameOrIpAddr": true}
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_pptp_username",
						"text": "Username",
						"validator": {"required": true}
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_pptp_password",
						"text": "Password",
						"validator": {"required": true}
					};
					c.addWidget(field);

					field = {
						"type": "checkbox",
						"name": "sys_iface_" + iface + "_pptp_defaultroute",
						"text": "Default route",
						"descr": "Add a default route to the system routing tables, using the peer as the gateway"
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_pptp_pppdopt",
						"text": "PPPD options",
						"defaultValue": "noauth nobsdcomp nodeflate nomppe"
					};
					c.addWidget(field);

					c.addSubmit();

					break;

				case "bonding":
					c.addTitle("Bonding Specific parameters");

					field = {
						"type": "text",
						"name": $.sprintf("sys_iface_%s_bond_ifaces", iface),
						"text": "Interfaces",
						"descr": "Interfaces for bonding separated by space.",
						"tip": "<b>Example:</b>eth0 eth1 dsl0<br><b>Note:</b>You can use only Ethernet-like" +
							" interfaces, like ethX, dslX, bondX<br><b>Note:</b> Interfaces should be" +
							" enabled, but <b>auto</b> should be switched <b>off</b>",
						"validator": {"required": true}
					};
					c.addWidget(field);

					/* set auto=0 enabled=1 for depending interfaces */
					var additionalKeys = [];
					c.addSubmit({
						"additionalKeys": additionalKeys,
						"preSubmit": function() {
							$.each($($.sprintf("#sys_iface_%s_bond_ifaces", iface)).val().split(" "),
								function(num, value) {
									$.addObjectWithProperty(additionalKeys, $.sprintf("sys_iface_%s_auto", value), "0");
									$.addObjectWithProperty(additionalKeys, $.sprintf("sys_iface_%s_enabled", value), "1");
								}
							);
						}
					});

					break;

				case "bridge":
					c.addTitle("Bridge Specific parameters");

					field = {
						"type": "checkbox",
						"name": "sys_iface_" + iface + "_br_stp",
						"text": "STP enabled",
						"descr": "Enable Spanning Tree Protocol.",
						"tip": "Multiple ethernet bridges can work together to create even larger networks" +
							" of ethernets using the IEEE 802.1d spanning tree protocol.This protocol is" +
							" used for finding the shortest path between two ethernets, and for eliminating" +
							" loops from the topology."
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_br_ifaces",
						"text": "Interfaces",
						"descr": "Interfaces for bridge separated by space.",
						"tip": "<b>Example:</b> eth0 eth1 dsl0<br><b>Note:</b> You can use only" +
						" Ethernet-like interfaces, like ethX, dslX<br><b>Note:</b> Interfaces should" +
						" be enabled, but <b>auto</b> should be switched <b>off</b>.",
						"validator": {"required": true}
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_br_prio",
						"text": "Priority",
						"descr": "Bridge priority.",
						"tip": "The priority value is an unsigned 16-bit quantity (a number between 0" +
							" and 65535), and has no dimension. Lower priority values are better. The bridge" +
							" with the lowest priority will be elected <b>root bridge</b>.",
						"validator": {"min": 1, "max": 65535}
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_br_fd",
						"text": "Forward delay",
						"descr": "Forward delay in seconds.",
						"validator": {"min": 0, "max": 60}
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_br_hello",
						"text": "Hello time",
						"descr": "Hello time in seconds",
						"validator": {"min": 0, "max": 60}
					};
					c.addWidget(field);

					field = {
						"type": "text",
						"name": "sys_iface_" + iface + "_br_maxage",
						"text": "Max age",
						"descr": "Max age in seconds",
						"validator": {"min": 0, "max": 600}
					};
					c.addWidget(field);

					/* set auto=0 enabled=1 for depending interfaces */
					var additionalKeys = [];
					c.addSubmit({
						"additionalKeys": additionalKeys,
						"preSubmit": function() {
							$.each($($.sprintf("#sys_iface_%s_br_ifaces", iface)).val().split(" "),
								function(num, value) {
									$.addObjectWithProperty(additionalKeys, $.sprintf("sys_iface_%s_auto", value), "0");
									$.addObjectWithProperty(additionalKeys, $.sprintf("sys_iface_%s_enabled", value), "1");
								}
							);
						}
					});

					break;
			}
		}
	});

	/* ROUTES tab */
	page.addTab({
		"id": "routes",
		"name": "Routes",
		"func": function() {
			var c = page.addContainer("routes");
			c.setHelpPage("traffic");
			c.setHelpSection("routes");
			setInfoText(c);

			/* create list of routes */
			var list = c.createList({
				"tabId": "routes",
				"header": ["Network", "Mask", "Gateway"],
				"varList": ["net", "netmask", "gw"],
				"listItem": $.sprintf("sys_iface_%s_route_", iface),
				"addMessage": "Add route",
				"editMessage": "Edit route",
				"listTitle": "Routes",
				"helpPage": "traffic",
				"helpSection": "routes.list"
			});

			field = {
				"type": "text",
				"name": "net",
				"text": "Network",
				"descr": "Network (without mask) or host",
				"tip": "E.g., 192.168.0.0 or 10.0.0.1",
				"validator": {"required": true, "ipAddr": true}
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "netmask",
				"text": "Netmask",
				"descr": "Netmask for network or host (in xxx.xxx.xxx.xxx format)",
				"tip": "E.g., 255.255.255.0 - /24 - Class C network<br>255.255.255.252 - /30" +
						"<br>255.255.255.255 - /32 - for a single host",
				"validator": {"required": true, "netmask": true}
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "gw",
				"text": "Gateway",
				"descr": "Gateway for route",
				"validator": {"required": true, "ipAddr": true}
			};
			list.addWidget(field);

			list.generateList();
		}
	});

	/* QoS tab */
	var showQos = function() {
		/* add widgets for PFIFO and BFIFO */
		var addFifoWidgets = function(type) {
			var field;
			field = {
				"type": "text",
				"name": $.sprintf("sys_iface_%s_qos_fifo_limit", iface),
				"text": "Buffer size",
				"descr": $.sprintf("Queue length in %s", type == "pfifo" ? "packets" : "bytes"),
				"validator": {"required": true, "min": 1, "max": 65535},
				"defaultValue": type == "pfifo" ? "128" : "10240"
			};
			c.addWidget(field);
		};

		/* add widgets for ESFQ */
		var addEsfqWidgets = function() {
			var field;
			field = {
				"type": "text",
				"name": $.sprintf("sys_iface_%s_qos_esfq_limit", iface),
				"text": "Limit",
				"descr": "Maximum packets in buffer",
				"validator": {"required": true, "min": 10, "max": 65535},
				"defaultValue": "128"
			};
			c.addWidget(field);

			field = {
				"type": "text",
				"name": $.sprintf("sys_iface_%s_qos_esfq_depth", iface),
				"text": "Depth",
				"validator": {"required": true, "min": 10, "max": 65535},
				"defaultValue": "128"
			};
			c.addWidget(field);

			field = {
				"type": "select",
				"name": $.sprintf("sys_iface_%s_qos_esfq_hash", iface),
				"text": "Hash",
				"options": {"classic": "Classic", "src": "Source address",
					"dst": "Destination address"},
				"defaultValue": "128"
			};
			c.addWidget(field);
		};

		/* add widgets for TBF */
		var addTbfWidgets = function() {
			var field;

			field = {
				"type": "text",
				"name": $.sprintf("sys_iface_%s_qos_tbf_rate", iface),
				"text": "Rate",
				"descr": "Maximum rate for interface",
				"validator": {"required": true, "qosBandwith": true},
				"defaultValue": "512kbit",
				"tip": "Unit can be: <br><i>kbit</i>, <i>Mbit</i> - for bit per second<br>" +
					"and <i>kbps</i>, <i>Mbps</i> - for bytes per second"
			};
			c.addWidget(field);
		};

		/* add widgets for HTB */
		var addHtbWidgets = function() {
			var field;

			/* generate list of available default classes */
			var defaultClasses = {"0": "none"};
			$.each(config.getParsed($.sprintf("sys_iface_%s_qos_htb_class_*", iface)),
				function(key, value) {
					var classId = value.classid.split(":")[1];
					defaultClasses[classId] = value.name;
				}
			);

			field = {
				"type": "select",
				"name": $.sprintf("sys_iface_%s_qos_htb_default", iface),
				"text": "Default class",
				"descr": "Name of default class",
				"options": defaultClasses,
				"defaultValue": "0"
			};
			c.addWidget(field);

			/*
			 * Callback for generateList(), returns name of parent class.
			 *
			 * data - hash with current key's variable info.
			 */
			var getParentName = function(data) {
				/* if current variable is not "parent" — return without modification */
				if (data.varName != "parent" && data.varName != "flowid") {
					return data.varValue;
				}

				/* if value of variable is "1:0" — class name is root */
				if (data.varValue == "1:0") {
					return "root";
				}

				/* search class with classid varValue and saves it's name to parentName */
				var parentName = "ERROR";
				var classes = config.getParsed($.sprintf("sys_iface_%s_qos_htb_class_*", iface));
				$.each(classes, function(classKey, classValues) {
					if (classValues.classid == data.varValue) {
						parentName = classValues.name;
						return false;
					}
				});

				return parentName;
			};

			/* add Classes table */
			page.addBr("qos");

			/* we use different variable because "c" is used to add elements to main form */
			var c2 = page.addContainer("qos");
			c2.setHelpPage("htb");
			c2.setHelpSection("htb");

			/* create list of classes */
			var list = c2.createList({
				"tabId": "qos",
				"header": ["Name", "Parent", "Rate", "Ceil", "Qdisc"],
				"varList": ["name", "parent", "rate", "ceil", "qdisc"],
				"listItem": $.sprintf("sys_iface_%s_qos_htb_class_", iface),
				"processValueFunc": getParentName,
				"addMessage": "Add QoS HTB class",
				"editMessage": "Edit QoS HTB class",
				"listTitle": $.sprintf("Classes on %s", iface),
				"helpPage": "htb",
				"helpSection": "htb_class_add"
			});

			field = {
				"type": "checkbox",
				"name": "enabled",
				"text": "Enabled",
				"descr": "Check this item to enable class"
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "name",
				"text": "Name",
				"descr": "Name of class",
				"validator": {"required": true, "alphanumU": true}
			};
			list.addWidget(field);

			/* generate list of available parent classes and find next classid */
			var max = 0;
			var parentClasses = {"1:0": "root"};
			$.each(config.getParsed($.sprintf("sys_iface_%s_qos_htb_class_*", iface)),
				function(key, value) {
					parentClasses[value['classid']] = value['name'];
					var cur = parseInt(value['classid'].split(":")[1], 10);
					if (cur > max) max = cur;
				}
			);
			var classid = "1:" + (max + 1);

			/*
			 * if we are editing class — it's classid will be get from KDB,
			 * if we are adding new class — we generate new classid and set it as default value.
			 */
			field = {
				"type": "hidden",
				"name": "classid",
				"defaultValue": classid
			};
			list.addWidget(field);

			field = {
				"type": "select",
				"name": "parent",
				"text": "Parent class",
				"descr": "Name of parent class",
				"options": parentClasses
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "rate",
				"text": "Rate",
				"descr": "Class rate",
				"tip": "Unit can be: <br><i>kbit</i>, <i>Mbit</i> - for bit per second<br>" +
					"and <i>kbps</i>, <i>Mbps</i> - for bytes per second.",
				"validator": {"required": true, "qosBandwith": true}
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "ceil",
				"text": "Ceil",
				"descr": "Max rate",
				"tip": "Unit can be: <br><i>kbit</i>, <i>Mbit</i> - for bit per second<br>" +
					"and <i>kbps</i>, <i>Mbps</i> - for bytes per second.",
				"validator": {"qosBandwith": true}
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "qdisc",
				"text": "Qdisc",
				"descr": "Qdisc for this class",
				"tip": "Optional qdisc for this class.<br><i>ATTENTION! At this moment, " +
					"you have to use character '#' for spaces.</i><br>For example, you can enter " +
					"<i>esfq#limit#128#depth#128#divisor#10#hash#classic#perturb#15</i> " +
					"or <i>sfq#perturb#10</i>, etc."
			};
			list.addWidget(field);

			list.generateList();

			/* add Filter table */
			page.addBr("qos");

			/* we use different variable because "c" is used to add elements to main form */
			c2 = page.addContainer("qos");
			c2.setHelpPage("htb");
			c2.setHelpSection("htb");

			/* create list of filter */
			list = c2.createList({
				"tabId": "qos",
				"header": ["Name", "Prio", "Proto", "Src addr", "Dst addr", "Src port", "Dst port", "Class"],
				"varList": ["name", "prio", "proto", "src", "dst", "src_port", "dst_port", "flowid"],
				"listItem": $.sprintf("sys_iface_%s_qos_htb_filter_", iface),
				"processValueFunc": getParentName,
				"addMessage": "Add QoS HTB filter",
				"editMessage": "Edit QoS HTB filter",
				"listTitle": $.sprintf("Filters on %s", iface),
				"helpPage": "htb",
				"helpSection": "htb_filter_add"
			});

			field = {
				"type": "checkbox",
				"name": "enabled",
				"text": "Enabled",
				"descr": "Check this item to enable class"
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "name",
				"text": "Name",
				"descr": "Name of filter",
				"validator": {"required": true, "alphanumU": true}
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "prio",
				"text": "Prio",
				"descr": "Rule priority",
				"tip": "Prio can be any positive integer value.<br><i>Examples:</i> 1, 10, 17.",
				"validator": {"required": true, "min": 1, "max": 65535},
				"defaultValue": "1"
			};
			list.addWidget(field);

			field = {
				"type": "select",
				"name": "proto",
				"text": "Protocol",
				"descr": "A protocol of the packet to check",
				"defaultValue": "any",
				"options": "any tcp udp icmp"
			};
			list.addWidget(field);

			var tip = "Address can be either a network IP address (with /mask), or a plain IP address, " +
				"A ! argument before the address specification inverts the sense of the address." +
				"<br><b>Examples:</b> 192.168.1.0/24, 192.168.1.5<br> Use 0.0.0.0/0 for <b>any</b>";

			field = {
				"type": "text",
				"name": "src",
				"text": "Source IP",
				"descr": "Source address",
				"validator": {"required": true, "ipNetMaskIptables": true},
				"defaultValue": "0.0.0.0/0",
				"tip": tip
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "dst",
				"text": "Destination IP",
				"descr": "Destination address",
				"validator": {"required": true, "ipNetMaskIptables": true},
				"defaultValue": "0.0.0.0/0",
				"tip": tip
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "src_port",
				"text": "Source port",
				"descr": "Source port",
				"validator": {"required": true, "ipPort": true},
				"defaultValue": "any"
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "dst_port",
				"text": "Destination port",
				"descr": "Destination port",
				"validator": {"required": true, "ipPort": true},
				"defaultValue": "any"
			};
			list.addWidget(field);

			/* generate list of available classes */
			var classes = new Object();
			$.each(config.getParsed($.sprintf("sys_iface_%s_qos_htb_class_*", iface)),
				function(key, value) {
					classes[value['classid']] = value['name'];
				}
			);

			field = {
				"type": "select",
				"name": "flowid",
				"text": "Class",
				"descr": "Put matching packets in this class",
				"options": classes
			};
			list.addWidget(field);

			list.generateList();
		};

		/* add widgets specific for selected scheduler */
		var addSchedulerWidgets = function() {
			var sch = $("#sch").val();
			switch (sch) {
				case "bfifo":
				case "pfifo":
					addFifoWidgets(sch);
					break;

				case "esfq":
					addEsfqWidgets();
					break;

				case "tbf":
					addTbfWidgets();
					break;

				case "htb":
					addHtbWidgets();
					break;
			}
		};

		/*
		 * on scheduler change remove unnecessary widgets (all except scheduler select)
		 * and add new widgets.
		 */
		var onSchedulerUpdate = function() {
			/* get tab object */
			var tab = page.getTab("qos");

			/* remove all widgets except scheduler select and txqueuelen */
			$("tbody > tr", tab).not("tr:has(#sch)").not("tr:has(#txqueuelen)").remove();

			/* remove all forms except first one (with scheduler select and txqueuelen) */
			$("form:not(:first)", tab).remove();

			/* remove all br between forms */
			$("form:first ~ br", tab).remove();

			/* add new widgets */
			addSchedulerWidgets();
		};

		/* add main form */
		var c, field;
		page.clearTab("qos");
		c = page.addContainer("qos");
		c.setHelpPage("qos");
		c.addTitle("QoS settings");
		setInfoText(c);

        /* txqueuelen */
        field = {
            "type": "text",
            "name": $.sprintf("sys_iface_%s_qos_txqueuelen", iface),
            "id": "txqueuelen",
            "text": "TX queue length",
            "descr": "Transmit queue length in packets. If empty, default value will be used.",
            "validator": {"min": 0, "max": 999999999}
        };
        c.addWidget(field);

		/* Scheduler */
		field = {
			"type": "select",
			"name": $.sprintf("sys_iface_%s_qos_sch", iface),
			"id": "sch",
			"text": "Scheduler",
			"descr": "Scheduler for the interface",
			"options": {
				"pfifo_fast": "Default discipline pfifo_fast",
				"bfifo": "FIFO with bytes buffer",
				"pfifo": "FIFO with packets buffer",
				"tbf": "Token Bucket Filter",
				"sfq": "Stochastic Fairness Queueing",
				"esfq": "Enhanced Stochastic Fairness Queueing",
				"htb": "Hierarchical Token Bucket"
			},
			"onChange": onSchedulerUpdate,
			"defaultValue": "pfifo_fast"
		};
		c.addWidget(field);

		addSchedulerWidgets();

		c.addSubmit();
	}

	page.addTab({
		"id": "qos",
		"name": "QoS",
		"func": showQos
	});

	/* DHCP tab */
	page.addTab({
		"id": "dhcp",
		"name": "DHCP",
		"func": function() {
			serviceDHCP(page, iface);
		}
	});

	page.generateTabs();
};
