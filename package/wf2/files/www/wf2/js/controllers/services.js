Controllers.dhcp = function() {
	var page = this.Page();
	
	/* DHCP tab */
	page.addTab({
		"id": "dhcp",
		"name": "DHCP server",
		"func": function() {
			var c, field;
			c = page.addContainer("dhcp");
			c.addTitle("DHCP server interface select");
			
			var onInterfaceChange = function() {
				/* save selected interface in cookie */
				$.cookie("dhcpInterface", $("#dhcpInterface").val());
				
				/* remove br and all forms (except first) with DHCP settings */
				$("form:eq(1) ~ br, form:eq(0) ~ form").remove();
				
				/* add DHCP settings for selected interface */
				serviceDHCP(page, $("#dhcpInterface").val());
			};
			
			/* Generate list of interfaces which can use DHCP */
			var ifaces = "";
			$.each(config.getParsed("sys_ifaces"), function(num, iface) {
				var proto = config.get($.sprintf("sys_iface_%s_proto", iface));
				switch (proto) {
					case "ether":
					case "bridge":
					case "bonding":
					case "vlan":
						ifaces += iface + " ";
						break;
				}
			});
			ifaces = $.trim(ifaces);
			
			/* value of this widget is saved in cookie, because we not need it in KDB */
			field = { 
				"type": "select",
				"name": "dhcpInterface",
				"id": "dhcpInterface",
				"cookie": true,
				"text": "Interface",
				"descr": "Select network interface",
				"options": ifaces,
				"onChange": onInterfaceChange
			};
			c.addWidget(field);
			
			page.addBr("dhcp");
			
			onInterfaceChange();
		}
	});
	
	page.generateTabs();
};

/*
 * Adds DHCP settings for specified interface.
 * 
 * page — destination page;
 * iface — interface.
 */
function serviceDHCP(page, iface) {
	var c, field;
	
	c = page.addContainer("dhcp");
	c.setHelpSection("dhcp_server");
	c.addTitle("DHCP server on interface " + iface);
	c.setSubsystem("dhcp." + iface);
	
	field = { 
		"type": "checkbox",
		"name": $.sprintf("sys_iface_%s_dhcp_enabled", iface),
		"id": "dhcpEnabled",
		"text": "Enable DHCP server",
		"descr": "Run DHCP server on interface " + iface
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_startip", iface),
		"text": "Start IP",
		"descr": "Start of dynamic IP address range for your LAN",
		"validator": {"required": true, "ipAddr": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_endip", iface),
		"text": "End IP",
		"descr": "End of dynamic IP address range for your LAN",
		"validator": {"required": true, "ipAddr": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_netmask", iface),
		"text": "Netmask",
		"descr": "Netmask for your LAN",
		"tip": "E.g., <i>255.255.255.0</i>",
		"validator": {"required": true, "netmask": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_router", iface),
		"text": "Default router",
		"descr": "Default router for your LAN hosts",
		"validator": {"ipAddr": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "select",
		"name": $.sprintf("sys_iface_%s_dhcp_lease_time", iface),
		"text": "Default lease time",
		"options": {"600": "10 minutes", "1800": "30 minutes", "3600": "1 hour", "10800": "3 hours",
			"36000": "10 hours", "86400": "24 hours"}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_nameserver", iface),
		"text": "DNS server",
		"descr": "DNS server for your LAN hosts",
		"validator": {"ipAddr": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_domain_name", iface),
		"text": "Domain",
		"descr": "Allows DHCP hosts to have fully qualified domain names",
		"tip": "Most queries for names within this domain can use short names relative to the local domain",
		"validator": {"domainName": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_ntpserver", iface),
		"text": "NTP server",
		"descr": "NTP server for your LAN hosts",
		"validator": {"ipAddr": true}
	};
	c.addWidget(field);
	
	field = { 
		"type": "text",
		"name": $.sprintf("sys_iface_%s_dhcp_winsserver", iface),
		"text": "WINS server",
		"descr": "WINS server for your LAN hosts",
		"validator": {"ipAddr": true}
	};
	c.addWidget(field);
	
	c.addSubmit();
	
	page.addBr("dhcp");
	serviceDHCPStatic(page, iface)
}

/*
 * Adds DHCP static lease table. ID of destination tab MUST BE "dhcp".
 * 
 * page — destination page;
 * iface — interface.
 */
function serviceDHCPStatic(page, iface) {
	var field;
	
	var c = page.addContainer("dhcp");
	c.setSubsystem("dhcp." + iface);
	
	/* create list of routes */
	var list = c.createList({
		"tabId": "dhcp",
		"header": ["Name", "IP address", "MAC address"],
		"varList": ["name", "ipaddr", "hwaddr"],
		"listItem": $.sprintf("sys_iface_%s_dhcp_host_", iface),
		"addMessage": "Add static lease",
		"editMessage": "Edit static lease",
		"listTitle": "DHCP static addresses on interface " + iface,
		"helpPage": "dhcp_server",
		"helpSection": "dhcp_server.static_add",
		"subsystem": "dhcp." + iface
	});
	
	field = { 
		"type": "text",
		"name": "name",
		"text": "Host name",
		"validator": {"required": true, "alphanumU": true}
	};
	list.addWidget(field);
	
	field = { 
		"type": "text",
		"name": "ipaddr",
		"text": "IP Address",
		"descr": "IP Address for host",
		"validator": {"required": true, "ipAddr": true}
	};
	list.addWidget(field);
	
	field = { 
		"type": "text",
		"name": "hwaddr",
		"text": "MAC Address",
		"descr": "MAC Address of host",
		"validator": {"required": true, "macAddr": true}
	};
	list.addWidget(field);
	
	list.generateList();
};

Controllers.dns_server = function() {
	var page = this.Page();
	page.setSubsystem("dns_server");
	page.setHelpPage("dns_server");

	/* settings tab */
	page.addTab({
		"id": "settings",
		"name": "Settings",
		"func": function() {
			var c, field;
			c = page.addContainer("settings");
			c.addTitle("DNS settings");

			field = {
				"type": "checkbox",
				"name": "svc_dns_enabled",
				"text": "Enable DNS server",
				"descr": "Check this item if you want to use DNS server on your router"
			};
			c.addWidget(field);

			field = {
				"type": "text",
				"name": "svc_dns_forwarder1",
				"text": "Forwarder DNS 1",
				"descr": "Forward queries to DNS server",
				"validator": {"ipAddr": true}
			};
			c.addWidget(field);

			field = {
				"type": "text",
				"name": "svc_dns_forwarder2",
				"text": "Forwarder DNS 2",
				"descr": "Forward queries to DNS server",
				"validator": {"ipAddr": true}
			};
			c.addWidget(field);

			c.addSubmit();
		}
	});

	/*
	 * Event handler.
	 * When adding or editing zone's record: when selected record type MX, add prio widget.
	 *
	 * list — current list to add new widget to (passed automatically by framework).
	 */
	var onChangeRecordType = function(list) {
		if ($("#datatype").val() == "MX" && $("#prio").length == 0) {
			/* add new field */
			var field = {
				"type": "text",
				"name": "prio",
				"text": "Priority",
				"descr": "Priority for MX record.",
				"validator": {"required": true, "min": 1, "max": 999}
			};
			list.addDynamicWidget(field,
				{"type": "insertAfter", "anchor": $("#data").parents("tr")});
		/* remove field */
		} else {
			$("#prio").parents("tr").remove();
		}
	};

	/*
	 * Page for manipulation with zone's records:
	 *  - show list of records;
	 *  - provides adding/editing/deleting capabilities.
	 *
	 * c — container where to show page;
	 * zoneid — ID of zone.
	 */
	var zoneRecordsPage = function(c, zoneid) {
		var field;

		/* clear this container */
		c.initContainer({"clear": true});

		/* create list of zone's records */
		var list = c.createList({
			"tabId": "zones",
			"header": ["Domain", "Type", "Data", "Priority"],
			"varList": ["domain", "datatype", "data", "prio"],
			"listItem": $.sprintf("svc_dns_zone_%s_", zoneid),
			"showPage": function() {
				zoneRecordsPage(c, zoneid);
			},
			"onAddOrEditItemRender": onChangeRecordType,
			"addMessage": $.sprintf("Add record to %s zone", zoneid),
			"editMessage": $.sprintf("Edit record in %s zone", zoneid),
			"listTitle": $.sprintf("Records for %s zone", zoneid),
			"helpPage": "dns_server",
			"helpSection": "dns_server.zone_record_add"
		});

		field = {
			"type": "select",
			"name": "datatype",
			"text": "Type of record",
			"descr": "Select type of record.",
			"options": "A CNAME MX NS PTR TXT",
			"onChange": onChangeRecordType
		};
		list.addWidget(field);

		field = {
			"type": "text",
			"name": "domain",
			"text": "Domain or host",
			"descr": "Enter domain name or host name.",
			"validator": {"required": true, "dnsRecordDomainOrIpAddr": true},
			"tip": "Use @ for current zone."
		};
		list.addWidget(field);

		field = {
			"type": "text",
			"name": "data",
			"text": "Data",
			"descr": "Data of the record.",
			"validator": {"required": true, "domainNameOrIpAddr": true},
			"tip": "If the record points to an EXTERNAL server (not defined in this zone) it MUST " +
				"end with a <i>.</i> (dot), e.g. ns1.example.net. If the name server is defined in " +
				"this domain (in this zone file) it can be written as ns1 (without the dot)."
		};
		list.addWidget(field);

		list.generateList();

		/* create button for returning back to the list of zones */
		field = {
			"type": "button",
			"name": "back_button",
			"text": "Back to list of zones",
			"cssName": "button",
			"func": function() {
				$("#tab_zones_link").click();
			}
		};
		c.addSubWidget(field, {"type": "appendToForm"});
	};

	/* zones tab */
	page.addTab({
		"id": "zones",
		"name": "Zones",
		"func": function() {
			var c, field;
			c = page.addContainer("zones");
			c.setHelpSection("dns_server.dns_zones");

			/* create list of zones */
			var list = c.createList({
				"tabId": "zones",
				"header": ["ID", "Name", "Admin", "Serial"],
				"varList": ["zoneid", "zone", "admin", "serial"],
				"varFunctions": {
					/* on user click on cell with zone ID show page with records for that zone */
					"zoneid": {
						"func": function(zoneid) {
							zoneRecordsPage(c, zoneid);
						},
						"tip": "Click to edit this zone."
					}
				},
				"listItem": "svc_dns_zonelist_",
				"onEditItemRender": function() {
					/* make Zone ID readonly in editing page */
					$("#zoneid").attr("readonly", true);
				},
				"addMessage": "Add DNS zone",
				"editMessage": "Edit DNS zone",
				"listTitle": "Zones",
				"helpPage": "dns_server",
				"helpSection": "dns_server.zone_add"
			});

			field = {
				"type": "text",
				"name": "zoneid",
				"text": "Zone ID",
				"descr": "Identifier of zone - just a simple name",
				"validator": {"required": true, "alphanum": true},
				"tip": "E.g., <i>domain2</i>"
			};
			list.addWidget(field);

			field = {
				"type": "hidden",
				"name": "zonetype",
				"defaultValue": "master"
			};
			list.addWidget(field);

			field = {
				"type": "checkbox",
				"name": "enabled",
				"text": "Enable",
				"descr": "Check this item to enable zone"
			};
			list.addWidget(field);

			/* generate zone's serial */
			var date = new Date();
			field = {
				"type": "text",
				"name": "serial",
				"text": "Serial",
				"descr": "Serial number of zone",
				"validator": {"required": true, "min": 1},
				"tip": "Common practice is to use as serial number date of last modification. " +
					"E.g., <i>2008110601</i> means year 2008, month 11, day 06 and day edition 01.",
				"defaultValue": "" + date.getFullYear()
					+ (date.getMonth() + 1 < 10 ? "0" + (date.getMonth() + 1) : (date.getMonth() + 1))
					+ (date.getDate() < 10 ? "0" + date.getDate() : date.getDate())
					+ "01"
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "zone",
				"text": "Zone",
				"descr": "Name of zone",
				"validator": {"required": true, "domainName": true},
				"tip": "E.g., <i>domain2.org</i>"
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "nameserver",
				"text": "Name server",
				"descr": "A name server that will respond authoritatively for the domain",
				"validator": {"required": true, "domainName": true},
				"tip": "This is most commonly written as a Fully-qualified Domain Name " +
					"(FQDN and ends with a dot). If the record points to an EXTERNAL server " +
					"(not defined in this zone) it MUST end with a . (dot) e.g. ns1.example.net."
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "admin",
				"text": "Admin",
				"descr": "Email of zone admin",
				"validator": {"required": true, "email": true}
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "refresh",
				"text": "Refresh",
				"descr": "Time (in seconds) when the slave will try to refresh the zone from the master.",
				"validator": {"required": true, "min": 1200, "max": 500000},
				"defaultValue": "28800",
				"tip": "Indicates the time when the slave will try to refresh the zone from the " +
					"master.<br>RFC 1912 recommends 1200 to 43200 seconds."
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "ttl",
				"text": "TTL",
				"descr": "Time (in seconds) to live.",
				"validator": {"required": true, "min": 1, "max": 500000},
				"defaultValue": "86400",
				"tip": "TTL in the DNS context defines the duration in seconds that the record " +
					"may be cached. Zero indicates the record should not be cached."
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "retry",
				"text": "Retry",
				"descr": "Defines the time (seconds) between retries if the slave (secondary) " +
					"fails to contact the master when refresh (above) has expired.",
				"validator": {"required": true, "min": 180, "max": 20000},
				"defaultValue": "7200",
				"tip": "Typical values should be 180 (3 minutes) to 900 (15 minutes), or higher."
			};
			list.addWidget(field);

			field = {
				"type": "text",
				"name": "expire",
				"text": "Expire",
				"descr": "Indicates when (in seconds) the zone data is no longer authoritative.",
				"validator": {"required": true, "min": 10000, "max": 90000000},
				"defaultValue": "1209600",
				"tip": "Slave servers stop responding to queries for the zone when this time has " +
					"expired and no contact has been made with the master<br>RFC 1912 " +
					"recommends 1209600 to 2419200 seconds (2-4 weeks) to allow for major outages " +
					"of the master."
			};
			list.addWidget(field);

			list.generateList();
		}
	});

	page.generateTabs();
};
