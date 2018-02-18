#include <drivers/idt.h>
#include <drivers/isr.h>
#include <stdio.h>
#include <asm/ioports.h>

isr_t handlers[256];

void isr_install(){
	set_idt_gate(0, (uint32_t)isr0);
	set_idt_gate(1, (uint32_t)isr1);
	set_idt_gate(2, (uint32_t)isr2);
	set_idt_gate(3, (uint32_t)isr3);
	set_idt_gate(4, (uint32_t)isr4);
	set_idt_gate(5, (uint32_t)isr5);
	set_idt_gate(6, (uint32_t)isr6);
	set_idt_gate(7, (uint32_t)isr7);
	set_idt_gate(8, (uint32_t)isr8);
	set_idt_gate(9, (uint32_t)isr9);
	set_idt_gate(10, (uint32_t)isr10);
	set_idt_gate(11, (uint32_t)isr11);
	set_idt_gate(12, (uint32_t)isr12);
	set_idt_gate(13, (uint32_t)isr13);
	set_idt_gate(14, (uint32_t)isr14);
	set_idt_gate(15, (uint32_t)isr15);
	set_idt_gate(16, (uint32_t)isr16);
	set_idt_gate(17, (uint32_t)isr17);
	set_idt_gate(18, (uint32_t)isr18);
	set_idt_gate(19, (uint32_t)isr19);
	set_idt_gate(20, (uint32_t)isr20);
	set_idt_gate(21, (uint32_t)isr21);
	set_idt_gate(22, (uint32_t)isr22);
	set_idt_gate(23, (uint32_t)isr23);
	set_idt_gate(24, (uint32_t)isr24);
	set_idt_gate(25, (uint32_t)isr25);
	set_idt_gate(26, (uint32_t)isr26);
	set_idt_gate(27, (uint32_t)isr27);
	set_idt_gate(28, (uint32_t)isr28);
	set_idt_gate(29, (uint32_t)isr29);
	set_idt_gate(30, (uint32_t)isr30);
	set_idt_gate(31, (uint32_t)isr31);

	// Remap the PIC
	port_byte_out(0x20, 0x11);
	port_byte_out(0xA0, 0x11);
	port_byte_out(0x21, 0x20);
	port_byte_out(0xA1, 0x28);
	port_byte_out(0x21, 0x04);
	port_byte_out(0xA1, 0x02);
	port_byte_out(0x21, 0x01);
	port_byte_out(0xA1, 0x01);
	port_byte_out(0x21, 0x0);
	port_byte_out(0xA1, 0x0); 

	// Install the IRQs
	set_idt_gate(32, (uint32_t)irq0);
	set_idt_gate(33, (uint32_t)irq1);
	set_idt_gate(34, (uint32_t)irq2);
	set_idt_gate(35, (uint32_t)irq3);
	set_idt_gate(36, (uint32_t)irq4);
	set_idt_gate(37, (uint32_t)irq5);
	set_idt_gate(38, (uint32_t)irq6);
	set_idt_gate(39, (uint32_t)irq7);
	set_idt_gate(40, (uint32_t)irq8);
	set_idt_gate(41, (uint32_t)irq9);
	set_idt_gate(42, (uint32_t)irq10);
	set_idt_gate(43, (uint32_t)irq11);
	set_idt_gate(44, (uint32_t)irq12);
	set_idt_gate(45, (uint32_t)irq13);
	set_idt_gate(46, (uint32_t)irq14);
	set_idt_gate(47, (uint32_t)irq15);

	set_idt(); // Load with ASM
}

void isr_handler(registers_t r){
	printf("Received interrupt: %d\n",r.int_no);
	// INT 0x50: Dump register values
	if(r.int_no==0x10){
		printf("EAX: %d (0x%x), ",r.eax,r.eax);
		printf("EBX: %d (0x%x), ",r.ebx,r.ebx);
		printf("ECX: %d (0x%x), ",r.ecx,r.ecx);
		printf("EDX: %d (0x%x), ",r.edx,r.edx);
		printf("EDI: %d (0x%x), ",r.edi,r.edi);
		printf("ESI: %d (0x%x)\n",r.esi,r.esi);
		printf("\nEBP: %d (0x%x)\n",r.ebp,r.ebp);
		printf("ESP: %d (0x%x)\n",r.esp,r.esp);
		printf("EIP: %d (0x%x)\n",r.eip,r.eip);
		printf("CS: %d (0x%x), ",r.cs,r.cs);
		printf("DS: %d (0x%x), ",r.ds,r.ds);
		printf("SS: %d (0x%x)\n",r.ss,r.ss);
	}
}

void register_interrupt_handler(uint8_t n,isr_t h){
	handlers[n]=h;
}

void irq_handler(registers_t r){
	if(handlers[r.int_no]!=0){
		isr_t h=handlers[r.int_no];
		h(r);
	}
	/* After every interrupt we need to send an EOI to the PICs
	 * or they will not send another interrupt again */
	if(r.int_no >= 40) port_byte_out(0xA0,0x20); /* slave */
	port_byte_out(0x20,0x20); /* master */
}
