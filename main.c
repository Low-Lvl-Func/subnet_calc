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

cidr_addr_t get_base_cls(cidr_addr_t caddr) {
    cidr_addr_t base;

    // 1. Calculate classful prefix rounded down to nearest multiple of 8
    base.prefix = (caddr.prefix / 8) * 8;

    // 2. Generate bitmask to clear host bits
    // Note: Special check for prefix 0 to prevent 32-bit shift overflow UB
    uint32_t mask = (base.prefix == 0) ? 0 : (0xFFFFFFFFU << (32 - base.prefix));

    // 3. Apply mask to zero-out lower bits
    base.ip = caddr.ip & mask;

    return base;
}

inline int get_hosts_per_subnet(cidr_addr_t addr) {
    return (1 << (32 - addr.prefix)) - 2;
}

inline int get_subnets_nr(cidr_addr_t addr, cidr_addr_t base) {
    return (1 << (addr.prefix - base.prefix));
}

void print_caddr_str_wo_pref(cidr_addr_t addr) {
    char buff[CIDR_ADDR_MAX_STR_LEN] = { 0 };
    cidr_addr_to_str_r(addr, buff);
    buff[(int)strcspn(buff, "/")] = '\0';
    printf(buff);
}

void print_subnets(cidr_addr_t addr, int hps, int nr_subnets) {
    puts("Subnets:");
    puts("Id | Usable hosts range | Broadcast");
    for (int i = 1; i <= nr_subnets; i++) {
        printf("#%d ", i);
        print_caddr_str_wo_pref(addr);
        putchar(' ');
        addr.ip++;
        print_caddr_str_wo_pref(addr);
        putchar('-');
        addr.ip += (hps - 1);
        print_caddr_str_wo_pref(addr);
        putchar(' ');
        addr.ip++;
        print_caddr_str_wo_pref(addr);
        puts("");
        addr.ip++;
    }
}

signed main(void) {
	char input[] = "172.25.167.98/19";
	puts(input);
	cidr_addr_t caddr = cidr_addr_from_str(input);
    puts("--------------------------------------");
    printf("Original: "); deb_print_cidr_addr(caddr);
    cidr_addr_t base = get_base_cls(caddr);
    printf("Base class: "); deb_print_cidr_addr(base);
    int hps = get_hosts_per_subnet(caddr);
    printf("Hosts/subnet: %d\n", hps);
    int nr_subnets = get_subnets_nr(caddr, base);
    printf("Nr. subnets: %d\n", nr_subnets);
    print_subnets(base, hps, nr_subnets);
	return 0;
}