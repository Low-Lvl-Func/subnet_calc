#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CIDR_ADDR_MAX_STR_LEN 20

typedef struct {
	uint32_t ip;
	uint8_t prefix;
} cidr_addr_t;

cidr_addr_t cidr_addr_from_str(const char cidr_addr_str[CIDR_ADDR_MAX_STR_LEN]) {
	
}

void compute_subnets() {

}

int main(void) {
	char input[] = "192.168.32.19/28";
	puts(input);
	//cidr_addr_t caddr = cidr_addr_from_str(input);

	return 0;
}