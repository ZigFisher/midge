baudrates = new Array(230400,115200,57600,38400,28800,19200,14400,9600,7200,4800,3600,2400,1800,1200,600,300);

function freeList(l){
    
    while ( l.length > 0 ){
		//	 alert("remove " + l.options[l.length-1].value);
        try{
            l.options.remove(l.length-1);
        }catch(ex){
			//	     alert("Cannot remove elem l.length-1");
            l.options[l.length-1] = null;
    	    l.length--;
		};
    };
};
	
function fixed_rate_list(l,cur,rates){
    freeList(l);
	ind=0;
	ind_res=0;
	val_res=0;

    for(i=0;i<rates.length;i++){
    	el = new Option;
    	el.value = rates[i];
    	el.text = rates[i];
		el.selected = 0;
		
		if( cur>0 && Math.abs(rates[i]-cur) < Math.abs(val_res-cur) ){
			ind_res = i;
			val_res = rates[i];
		};
		try{
			l.options.add(el);
		} catch(ex){
			l.options.add(el,null);
		};
    };

	// Select current rate
	l.selectedIndex = ind_res;
	l[ind_res].selected = 1;
};




function OnChangeSerial()
{
	ind = $('baudrate').selectedIndex;

	if( ind < 0 || ind==undefined ){
		ind = 0;
		rate = 230400;
	}else{
		rate = $('baudrate').options[ind].value;
	}
	fixed_rate_list($('baudrate'),rate,baudrates);
};

