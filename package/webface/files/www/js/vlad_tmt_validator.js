// <Vlad>
// Filters
tmt_globalFilters.nobackquotes = tmt_filterInfo('`', "");
tmt_globalFilters.nomagic = tmt_filterInfo('[`$();]', "");

tmt_globalPatterns.A_z= new RegExp("^\([A-Za-z]+\)$");
tmt_globalPatterns.A_z0_9= new RegExp("^\([A-Za-z0-9]+\)$");
tmt_globalPatterns.A_z0_9_= new RegExp("^\([_A-Za-z0-9]+\)$");

// IP Addr validation
tmt_globalPatterns.ipaddr = new RegExp("^\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])$");
tmt_globalPatterns.ipaddr_asterisk = new RegExp("^\(\\*|\(\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5]))\)$");
// IP Addr range: 192.168.1.100-200
tmt_globalPatterns.ipaddr_range= new RegExp("^(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])(-(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5]))?$");
// Netmask validation
tmt_globalPatterns.netmask = new RegExp("^\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])$");
// MAC Address validation
tmt_globalPatterns.macaddr = new RegExp("^\([0-9a-fA-F][0-9a-fA-F]:\){5}\([0-9a-fA-F][0-9a-fA-F]\)$");
// IP net validation (ip/bitmask)
tmt_globalPatterns.ipnet  = new RegExp("^(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])(/\\d\\d*)?$");
tmt_globalPatterns.ipnet_ipt  = new RegExp("^[!]?(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])(/\\d\\d*)?$");
tmt_globalPatterns.ipaddrport = new RegExp("^(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])(\\:\\d*)?$");
tmt_globalPatterns.ipportrange = new RegExp("^((\\d)+(:)?((\\d)+)?|(:)((\\d)+)|any)$");
// IP port
tmt_globalPatterns.ipport = new RegExp("^((\\d)+|any)$");
// mxsmap or mxrate
tmt_globalPatterns.mxrate = new RegExp("^(([0-9]+,)|([0-9]+-[0-9]+(,)?)|([0-9]+$))+$");

// DNS Zone validation
tmt_globalPatterns.dnszone = new RegExp("^\([a-zA-Z0-9\-\.]\)+$");
// DNS Zone ID validation
tmt_globalPatterns.dnszoneid = new RegExp("^\([a-zA-Z0-9]\)+$");
// DNS Host validation
tmt_globalPatterns.dnsdomain = new RegExp("^\(\([a-zA-Z0-9\-\.]\)+|@\)$");

tmt_globalPatterns.dnsdomainoripaddr = new RegExp("^\(\([a-zA-Z0-9\-\.]\)+|\(\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5]))\)$");

tmt_globalPatterns.qoslatency = new RegExp("^\([0-9]+ms\)$");
tmt_globalPatterns.qosbandw = new RegExp("^\([0-9]+\(k|M\)\(bit|bps\)\)$");

// ipsec key validation
tmt_globalPatterns.ipsec_key = new RegExp("^\(0x[0-9a-fA-F]*\)$");
tmt_globalPatterns.ipsec_spi = new RegExp("^\(0x[0-9a-fA-F]+|[0-9]+\)$");
tmt_globalPatterns.ipsec_src_dst = new RegExp("((^$)|^(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])(\\[\\d+\\])?-(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])(\\[\\d+\\])?$)");

// voip
tmt_globalPatterns.voip_router_id = new RegExp("^([0-9]){3}$");
tmt_globalPatterns.voip_short_number = new RegExp("^([0-9]){2}$");
tmt_globalPatterns.voip_complete_number = new RegExp("^((([0-9]{3}|[\*]))((([-,]*)([0-9]{2}|[\*])([-,]*)([-0-9,]*))|(([-,]*)[#]sip:([_a-zA-Z0-9]\.?)+@([_a-zA-Z0-9]\.?)+[#])))$");
tmt_globalPatterns.voip_registrar = new RegExp("^sip:\(\([a-zA-Z0-9\-\.]\)+|\(\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5])\\.\(0?0?\\d|[01]?\\d\\d|2[0-4]\\d|25[0-5]))\)$");
tmt_globalPatterns.voip_sip_uri = new RegExp("^sip:([_a-zA-Z0-9]\.*)+@([_a-zA-Z0-9]\.?)+");

// </Vlad>
