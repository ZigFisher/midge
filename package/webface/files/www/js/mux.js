function OnChangeMuxCLKM(clkm)
{
	/* get interface name */
	iface = clkm.id.substring(clkm.id.indexOf("_") + 1, clkm.id.length);
	
	clkr = document.getElementById("clkr_" + iface);
	
	/* if interface is slave, disable CLKR for it, otherwise — enable */ 
	clkr.disabled = !clkm.selectedIndex;
}