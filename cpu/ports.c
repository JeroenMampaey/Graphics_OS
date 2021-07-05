unsigned char port_byte_in(unsigned short port){
	unsigned char result;
	/*
	copy value from IO-port specified by dx into al, 
	put value from al into result
	*/
	__asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
	return result;
}

void port_byte_out(unsigned short port, unsigned char data){
	/*
	 put data into al en port into dx and
	 out al into the IO-port specified by dx 
	 */
	__asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

unsigned short port_word_in(unsigned short port){
	unsigned short result;
	/*similar to port_byte_in*/
	__asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
	return result;
}

void port_word_out(unsigned short port, unsigned short data){
	/*similar to port_byte_out*/
	__asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}
