#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define CIDR_ADDR_MAX_STR_LEN 32

typedef struct {
    uint32_t ip;
    uint8_t prefix;
} cidr_addr_t;

// Helper function to handle parsing failures
static void fail_parse(const char* reason) {
    fprintf(stderr, "Error parsing CIDR string: %s\n", reason);
    exit(1);
}

cidr_addr_t cidr_addr_from_str(const char cidr_addr_str[CIDR_ADDR_MAX_STR_LEN]) {
    cidr_addr_t result = { 0, 0 };

    if (cidr_addr_str == NULL) {
        fail_parse("Input string is NULL");
    }

    // 1. Skip leading whitespace
    const char* p = cidr_addr_str + strspn(cidr_addr_str, " \t");

    // 2. Locate the '/' delimiter
    const char* slash = strpbrk(p, "/");
    if (slash == NULL) {
        fail_parse("Missing '/' separator");
    }

    // 3. Manually parse the 4 IPv4 octets
    uint32_t ip_acc = 0;
    int octet_count = 0;

    while (p < slash) {
        const char* next_delim = strpbrk(p, "./");
        if (next_delim == NULL || next_delim > slash) {
            next_delim = slash;
        }

        size_t len = (size_t)(next_delim - p);
        if (len == 0 || len > 3) {
            fail_parse("Invalid IP octet length");
        }

        uint32_t octet = 0;
        for (size_t i = 0; i < len; i++) {
            if (p[i] < '0' || p[i] > '9') {
                fail_parse("IP octet contains non-digit characters");
            }
            octet = octet * 10 + (uint32_t)(p[i] - '0');
        }

        if (octet > 255) {
            fail_parse("IP octet value out of range (> 255)");
        }

        ip_acc = (ip_acc << 8) | octet;
        octet_count++;

        p = next_delim;
        if (*p == '.') {
            p++;
        }
    }

    if (octet_count != 4) {
        fail_parse("IP address must consist of exactly 4 octets");
    }

    // 4. Manually parse the CIDR prefix after '/'
    p = slash + 1;
    size_t prefix_len = strspn(p, "0123456789");
    if (prefix_len == 0 || prefix_len > 2) {
        fail_parse("Invalid prefix length format");
    }

    // Ensure there are no trailing unexpected characters after the digits
    if (p[prefix_len] != '\0' && strchr(" \t\r\n", p[prefix_len]) == NULL) {
        fail_parse("Trailing invalid characters after prefix");
    }

    uint32_t prefix_acc = 0;
    for (size_t i = 0; i < prefix_len; i++) {
        prefix_acc = prefix_acc * 10 + (uint32_t)(p[i] - '0');
    }

    if (prefix_acc > 32) {
        fail_parse("CIDR prefix out of range (> 32)");
    }

    result.ip = ip_acc;
    result.prefix = (uint8_t)prefix_acc;

    return result;
}

char* cidr_addr_to_str_r(cidr_addr_t addr, char out_buf[CIDR_ADDR_MAX_STR_LEN]) {
    uint8_t o1 = (uint8_t)(addr.ip >> 24);
    uint8_t o2 = (uint8_t)(addr.ip >> 16);
    uint8_t o3 = (uint8_t)(addr.ip >> 8);
    uint8_t o4 = (uint8_t)(addr.ip);

    snprintf(out_buf, CIDR_ADDR_MAX_STR_LEN, "%u.%u.%u.%u/%u", o1, o2, o3, o4, addr.prefix);
    return out_buf;
}

void deb_print_cidr_addr(cidr_addr_t addr) {
    char aux[CIDR_ADDR_MAX_STR_LEN] = {0};
    puts(cidr_addr_to_str_r(addr, aux));
}

void compute_subnets() {

}

int main(void) {
	char input[] = "192.168.32.19/28";
	puts(input);
	cidr_addr_t caddr = cidr_addr_from_str(input);
    deb_print_cidr_addr(caddr);


	return 0;
}