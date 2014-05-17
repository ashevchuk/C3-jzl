#include <features.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
//#include <cfgfile.h>
#include <resolv.h>
#include <netdb.h>
#include <ctype.h>
#include <arpa/nameser.h>
#include <sys/fcntl.h>
#define MAX_RECURSE 5
#define REPLY_TIMEOUT 10
//#define MAX_RETRIES 15
#define MAX_RETRIES 3
#define MAX_SERVERS 3
#define MAX_SEARCH 4

//#undef DEBUG
#define DEBUG

#ifdef DEBUG
#define DPRINTF(X,args...) fprintf(stderr, X, ##args)
#else
#define DPRINTF(X,args...)
#endif /* DEBUG */

struct resolv_header {
	int id;
	int qr,opcode,aa,tc,rd,ra,rcode;
	int qdcount;
	int ancount;
	int nscount;
	int arcount;
};

struct resolv_question {
	char * dotted;
	int qtype;
	int qclass;
};

struct resolv_answer {
	char * dotted;
	int atype;
	int aclass;
	int ttl;
	int rdlength;
	unsigned char * rdata;
	int rdoffset;
};





extern int nameservers;
extern char * nameserver[MAX_SERVERS];
extern int searchdomains;
extern char * searchdomain[MAX_SEARCH];
extern struct hostent * get_hosts_byname(const char * name);
extern struct hostent * get_hosts_byaddr(const char * addr, int len, int type);
extern struct hostent * read_etc_hosts(const char * name, int ip);
extern int resolve_address(const char * address, int nscount,
	char ** nsip, struct in_addr * in);
extern int resolve_mailbox(const char * address, int nscount,
	char ** nsip, struct in_addr * in);
extern int dns_lookup(const char * name, int type, int nscount,
	char ** nsip, unsigned char ** outpacket, struct resolv_answer * a);

int findinhosts=0;

unsigned char name_arg[50];
int nameservers;
char * nameserver[MAX_SERVERS];
int searchdomains;
char * searchdomain[MAX_SEARCH];

//#ifdef L_encodeh
int encode_header(struct resolv_header *h, unsigned char *dest, int maxlen)
{
	if (maxlen < HFIXEDSZ)
		return -1;

	dest[0] = (h->id & 0xff00) >> 8;
	dest[1] = (h->id & 0x00ff) >> 0;
	dest[2] = (h->qr ? 0x80 : 0) |
		((h->opcode & 0x0f) << 3) |
		(h->aa ? 0x04 : 0) |
		(h->tc ? 0x02 : 0) |
		(h->rd ? 0x01 : 0);
	dest[3] = (h->ra ? 0x80 : 0) | (h->rcode & 0x0f);
	dest[4] = (h->qdcount & 0xff00) >> 8;
	dest[5] = (h->qdcount & 0x00ff) >> 0;
	dest[6] = (h->ancount & 0xff00) >> 8;
	dest[7] = (h->ancount & 0x00ff) >> 0;
	dest[8] = (h->nscount & 0xff00) >> 8;
	dest[9] = (h->nscount & 0x00ff) >> 0;
	dest[10] = (h->arcount & 0xff00) >> 8;
	dest[11] = (h->arcount & 0x00ff) >> 0;

	return HFIXEDSZ;
}
//#endif

//#ifdef L_decodeh
int decode_header(unsigned char *data, struct resolv_header *h)
{
	h->id = (data[0] << 8) | data[1];
	h->qr = (data[2] & 0x80) ? 1 : 0;
	h->opcode = (data[2] >> 3) & 0x0f;
	h->aa = (data[2] & 0x04) ? 1 : 0;
	h->tc = (data[2] & 0x02) ? 1 : 0;
	h->rd = (data[2] & 0x01) ? 1 : 0;
	h->ra = (data[3] & 0x80) ? 1 : 0;
	h->rcode = data[3] & 0x0f;
	h->qdcount = (data[4] << 8) | data[5];
	h->ancount = (data[6] << 8) | data[7];
	h->nscount = (data[8] << 8) | data[9];
	h->arcount = (data[10] << 8) | data[11];

	return HFIXEDSZ;
}
//#endif


/* Encode a dotted string into nameserver transport-level encoding.
   This routine is fairly dumb, and doesn't attempt to compress
   the data */

int encode_dotted(const char *dotted, unsigned char *dest, int maxlen)
{
	int used = 0;

	while (dotted && *dotted) {
		char *c = strchr(dotted, '.');
		int l = c ? c - dotted : strlen(dotted);

		if (l >= (maxlen - used - 1))
			return -1;

		dest[used++] = l;
		memcpy(dest + used, dotted, l);
		used += l;

		if (c)
			dotted = c + 1;
		else
			break;
	}

	if (maxlen < 1)
		return -1;

	dest[used++] = 0;

	return used;
}
//#endif


/* Decode a dotted string from nameserver transport-level encoding.
   This routine understands compressed data. */

int decode_dotted(const unsigned char *data, int offset,
				  char *dest, int maxlen)
{
	int l;
	int measure = 1;
	int total = 0;
	int used = 0;

	if (!data)
		return -1;

	while ((l=data[offset++])) {
		if (measure)
		    total++;
		if ((l & 0xc0) == (0xc0)) {
			if (measure)
				total++;
			/* compressed item, redirect */
			offset = ((l & 0x3f) << 8) | data[offset];
			measure = 0;
			continue;
		}

		if ((used + l + 1) >= maxlen)
			return -1;

		memcpy(dest + used, data + offset, l);
		offset += l;
		used += l;
		if (measure)
			total += l;

		if (data[offset] != 0)
			dest[used++] = '.';
		else
			dest[used++] = '\0';
	}

	DPRINTF("Total decode len = %d\n", total);

	return total;
}




int length_dotted(const unsigned char *data, int offset)
{
	int orig_offset = offset;
	int l;

	if (!data)
		return -1;

	while ((l = data[offset++])) {

		if ((l & 0xc0) == (0xc0)) {
			offset++;
			break;
		}

		offset += l;
	}

	return offset - orig_offset;
}



int encode_question(struct resolv_question *q,
					unsigned char *dest, int maxlen)
{
	int i;

	i = encode_dotted(q->dotted, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;

	if (maxlen < 4)
		return -1;

	dest[0] = (q->qtype & 0xff00) >> 8;
	dest[1] = (q->qtype & 0x00ff) >> 0;
	dest[2] = (q->qclass & 0xff00) >> 8;
	dest[3] = (q->qclass & 0x00ff) >> 0;

	return i + 4;
}



int decode_question(unsigned char *message, int offset,
					struct resolv_question *q)
{
	char temp[256];
	int i;

	i = decode_dotted(message, offset, temp, sizeof(temp));
	if (i < 0)
		return i;

	offset += i;

	q->dotted = strdup(temp);
	q->qtype = (message[offset + 0] << 8) | message[offset + 1];
	q->qclass = (message[offset + 2] << 8) | message[offset + 3];

	return i + 4;
}



int length_question(unsigned char *message, int offset)
{
	int i;

	i = length_dotted(message, offset);
	if (i < 0)
		return i;

	return i + 4;
}



int encode_answer(struct resolv_answer *a, unsigned char *dest, int maxlen)
{
	int i;

	i = encode_dotted(a->dotted, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;

	if (maxlen < (RRFIXEDSZ+a->rdlength))
		return -1;

	*dest++ = (a->atype & 0xff00) >> 8;
	*dest++ = (a->atype & 0x00ff) >> 0;
	*dest++ = (a->aclass & 0xff00) >> 8;
	*dest++ = (a->aclass & 0x00ff) >> 0;
	*dest++ = (a->ttl & 0xff000000) >> 24;
	*dest++ = (a->ttl & 0x00ff0000) >> 16;
	*dest++ = (a->ttl & 0x0000ff00) >> 8;
	*dest++ = (a->ttl & 0x000000ff) >> 0;
	*dest++ = (a->rdlength & 0xff00) >> 8;
	*dest++ = (a->rdlength & 0x00ff) >> 0;
	memcpy(dest, a->rdata, a->rdlength);

	return i + RRFIXEDSZ + a->rdlength;
}



int decode_answer(unsigned char *message, int offset,
				  struct resolv_answer *a)
{
	char temp[256];
	int i;

	i = decode_dotted(message, offset, temp, sizeof(temp));
	if (i < 0)
		return i;

	message += offset + i;

	a->dotted = strdup(temp);
	a->atype = (message[0] << 8) | message[1];
	message += 2;
	a->aclass = (message[0] << 8) | message[1];
	message += 2;
	a->ttl = (message[0] << 24) |
		(message[1] << 16) | (message[2] << 8) | (message[3] << 0);
	message += 4;
	a->rdlength = (message[0] << 8) | message[1];
	message += 2;
	a->rdata = message;
	a->rdoffset = offset + i + RRFIXEDSZ;
/*
	#if 1
		printf("domain = %s\n",a->dotted);
		
	#endif
*/
	DPRINTF("i=%d,rdlength=%d\n", i, a->rdlength);

	return i + RRFIXEDSZ + a->rdlength;
}



int encode_packet(struct resolv_header *h,
	struct resolv_question **q,
	struct resolv_answer **an,
	struct resolv_answer **ns,
	struct resolv_answer **ar,
	unsigned char *dest, int maxlen)
{
	int i, total = 0;
	int j;

	i = encode_header(h, dest, maxlen);
	if (i < 0)
		return i;

	dest += i;
	maxlen -= i;
	total += i;

	for (j = 0; j < h->qdcount; j++) {
		i = encode_question(q[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}

	for (j = 0; j < h->ancount; j++) {
		i = encode_answer(an[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}
	for (j = 0; j < h->nscount; j++) {
		i = encode_answer(ns[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}
	for (j = 0; j < h->arcount; j++) {
		i = encode_answer(ar[j], dest, maxlen);
		if (i < 0)
			return i;
		dest += i;
		maxlen -= i;
		total += i;
	}

	return total;
}



int decode_packet(unsigned char *data, struct resolv_header *h)
{
	return decode_header(data, h);
}


int dns_caught_signal = 0;
void dns_catch_signal(int signo)
{
	dns_caught_signal = 1;
}
int dns_lookup(const char *name, int type, int nscount, char **nsip,
			   unsigned char **outpacket, struct resolv_answer *a)
{
	static int id = 1;
	int i, j, len, fd, pos;
	static int ns = 0;
	struct sockaddr_in sa;
#ifdef __UCLIBC_HAS_IPV6__
	struct sockaddr_in6 sa6;
#endif /* __UCLIBC_HAS_IPV6__ */
	int oldalarm;
	__sighandler_t oldhandler;
	struct resolv_header h;
	struct resolv_question q;
	int retries = 0;
	unsigned char * packet = malloc(PACKETSZ);
	unsigned char * lookup = malloc(MAXDNAME);
	int variant = 0;
#ifdef __UCLIBC_HAS_IPV6__
	int v6;
#endif /* __UCLIBC_HAS_IPV6__ */

	fd = -1;

	if (!packet || !lookup || !nscount)
	    goto fail;

	DPRINTF("Looking up type %d answer for '%s'\n", type, name);

	ns %= nscount;

	while (retries++ < MAX_RETRIES) {
#ifdef __UCLIBC_HAS_IPV6__
		v6 = (inet_pton(AF_INET6, nsip[ns], &sa6.sin6_addr) > 0);
#endif /* __UCLIBC_HAS_IPV6__ */

		if (fd != -1)
			close(fd);

#ifndef __UCLIBC_HAS_IPV6__
		fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else /* __UCLIBC_HAS_IPV6__ */
		fd = socket(v6 ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif /* __UCLIBC_HAS_IPV6__ */

		if (fd == -1)
			goto fail;

		memset(packet, 0, PACKETSZ);

		memset(&h, 0, sizeof(h));
		h.id = ++id;
		h.qdcount = 1;
		h.rd = 1;


		i = encode_header(&h, packet, PACKETSZ);
		if (i < 0)
			goto fail;

		strncpy(lookup,name,MAXDNAME);
		if (variant < searchdomains && strchr(lookup, '.') == NULL)
		{
		    strncat(lookup,".", MAXDNAME);
		    strncat(lookup,searchdomain[variant], MAXDNAME);
		}
		DPRINTF("lookup name: %s\n", lookup);
		q.dotted = (char *)lookup;
		q.qtype = type;
		q.qclass = C_IN; /* CLASS_IN */

		j = encode_question(&q, packet+i, PACKETSZ-i);
		if (j < 0)
			goto fail;

		len = i + j;

		DPRINTF("On try %d, sending query to port %d of machine %s\n",
				retries, NAMESERVER_PORT, nsip[ns]);

#ifndef __UCLIBC_HAS_IPV6__
		sa.sin_family = AF_INET;
		sa.sin_port = htons(NAMESERVER_PORT);
		sa.sin_addr.s_addr = inet_addr(nsip[ns]);
#else /* __UCLIBC_HAS_IPV6__ */
		if (v6) {
			sa6.sin6_family = AF_INET6;
			sa6.sin6_port = htons(NAMESERVER_PORT);
			/* sa6.sin6_addr is already here */
		} else {
			sa.sin_family = AF_INET;
			sa.sin_port = htons(NAMESERVER_PORT);
			sa.sin_addr.s_addr = inet_addr(nsip[ns]);
		}
#endif /* __UCLIBC_HAS_IPV6__ */

#ifndef __UCLIBC_HAS_IPV6__
		if (connect(fd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
#else /* __UCLIBC_HAS_IPV6__ */
		if (connect(fd, (struct sockaddr *) (v6 ? &sa6 : &sa),
			    v6 ? sizeof(sa6) : sizeof(sa)) == -1) {
#endif /* __UCLIBC_HAS_IPV6__ */
			if (errno == ENETUNREACH) {
				/* routing error, presume not transient */
				goto tryall;
			} else
				/* retry */
				continue;
		}

		DPRINTF("Transmitting packet of length %d, id=%d, qr=%d\n",
				len, h.id, h.qr);

		send(fd, packet, len, 0);

		dns_caught_signal = 0;
		oldalarm = alarm(REPLY_TIMEOUT);
		oldhandler = signal(SIGALRM, dns_catch_signal);

/*

		int nNetTimeout=3000;	//set time out of resolv
		//发送时限
		DPRINTF("set time opt\n");
		setsockopt(fd,SOL_SOCKET, SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));
		DPRINTF("rev net resolv...\n");		
*/
		struct timeval timeout ;
		fd_set r;
		FD_ZERO(&r);
		FD_SET(fd, &r);
		timeout.tv_sec = 3; 			//15s timeout
		timeout.tv_usec =0;
		select(fd+1, &r, NULL, NULL, &timeout);//
		if(FD_ISSET(fd, &r))			//can read it
		{
			i = recv(fd, packet, PACKETSZ, 0);		//
		}
		
		alarm(0);
		signal(SIGALRM, oldhandler);
		alarm(oldalarm);

		DPRINTF("Timeout=%d, len=%d\n", dns_caught_signal, i);

		if (dns_caught_signal)
			/* timed out, so retry send and receive,
			   to next nameserver on queue */
			goto again;

		if (i < HFIXEDSZ)
			/* too short ! */
			goto again;

		decode_header(packet, &h);

		DPRINTF("id = %d, qr = %d\n", h.id, h.qr);

		if ((h.id != id) || (!h.qr))
			/* unsolicited */
			goto again;

		DPRINTF("Got response %s\n", "(i think)!");
		DPRINTF("qrcount=%d,ancount=%d,nscount=%d,arcount=%d\n",
				h.qdcount, h.ancount, h.nscount, h.arcount);
		DPRINTF("opcode=%d,aa=%d,tc=%d,rd=%d,ra=%d,rcode=%d\n",
				h.opcode, h.aa, h.tc, h.rd, h.ra, h.rcode);

		if ((h.rcode) || (h.ancount < 1)) {
			/* negative result, not present */
			goto again;
		}

		pos = HFIXEDSZ;

		for (j = 0; j < h.qdcount; j++) {
			DPRINTF("Skipping question %d at %d\n", j, pos);
			i = length_question(packet, pos);
			DPRINTF("Length of question %d is %d\n", j, i);
			if (i < 0)
				goto again;
			pos += i;
		}
		DPRINTF("Decoding answer at pos %d\n", pos);

		for (j=0;j<h.ancount;j++)
		{
		    i = decode_answer(packet, pos, a);

		    if (i<0) {
			DPRINTF("failed decode %d\n", i);
			goto again;
		    }
		    /* For all but T_SIG, accept first answer */
		    if (a->atype != T_SIG)
			break;
		//   if(strcmp(a->dotted, name_arg)==0)
		//	break;
		   DPRINTF("skipping T_SIG %d\n", i);
		   free(a->dotted);
		   pos += i;
		}

		DPRINTF("Answer name = |%s|\n", a->dotted);
		DPRINTF("Answer type = |%d|\n", a->atype);

		close(fd);

		if (outpacket)
			*outpacket = packet;
		else
			free(packet);
		return (0);				/* success! */

	  tryall:
		/* if there are other nameservers, give them a go,
		   otherwise return with error */
		variant = 0;
		if (retries >= nscount*(searchdomains+1))
		    goto fail;

	  again:
		/* if there are searchdomains, try them or fallback as passed */
		if (variant < searchdomains) {
		    /* next search */
		    variant++;
		} else {
		    /* next server, first search */
		    ns = (ns + 1) % nscount;
		    variant = 0;
		}
	}

fail:
	if (fd != -1)
	    close(fd);
	if (lookup)
	    free(lookup);
	if (packet)
	    free(packet);
	return -1;
}





struct hostent * read_etc_hosts(const char * name, int ip)
{
	static struct hostent	h;
	static struct in_addr	in;
	static struct in_addr	*addr_list[2];
#ifdef __UCLIBC_HAS_IPV6__
	static struct in6_addr	in6;
	static struct in6_addr	*addr_list6[2];
#endif /* __UCLIBC_HAS_IPV6__ */
	static char				line[80];
	FILE					*fp;
	char					*cp;
#define		 MAX_ALIAS		5
	char					*alias[MAX_ALIAS];
	int						aliases, i;

	if ((fp = fopen("/etc/hosts", "r")) == NULL &&
			(fp = fopen("/etc/config/hosts", "r")) == NULL)
		return((struct hostent *) NULL);

	while (fgets(line, sizeof(line), fp)) {
		if ((cp = strchr(line, '#')))
			*cp = '\0';
		aliases = 0;

		cp = line;
		while (*cp) {
			while (*cp && isspace(*cp))
				*cp++ = '\0';
			if (!*cp)
				continue;
			if (aliases < MAX_ALIAS)
				alias[aliases++] = cp;
			while (*cp && !isspace(*cp))
				cp++;
		}

		if (aliases < 2)
			continue; /* syntax error really */

		if (ip) {
			if (strcmp(name, alias[0]) != 0)
				continue;
		} else {
			for (i = 1; i < aliases; i++)
				if (strcasecmp(name, alias[i]) == 0)
					break;
			if (i >= aliases)
				continue;
		}

#ifndef __UCLIBC_HAS_IPV6__
		if (inet_aton(alias[0], &in) == 0)
			break; /* bad ip address */
#else /* __UCLIBC_HAS_IPV6__ */
		if (inet_aton(alias[0], &in) == 0) {
			if (inet_pton(AF_INET6, alias[0], &in6) == 0) {
				addr_list6[0] = &in6;
				addr_list6[1] = 0;
				h.h_name = alias[1];
				h.h_addrtype = AF_INET6;
				h.h_length = sizeof(in6);
				h.h_addr_list = (char**) addr_list6;
				fclose(fp);
				return(&h);
			} else
				break; /* bad ip address */
		}
#endif /* __UCLIBC_HAS_IPV6__ */

		addr_list[0] = &in;
		addr_list[1] = 0;
		h.h_name = alias[1];
		h.h_addrtype = AF_INET;
		h.h_length = sizeof(in);
		h.h_addr_list = (char**) addr_list;
		fclose(fp);
		return(&h);
	}
	fclose(fp);
	return((struct hostent *) NULL);
}



struct hostent * get_hosts_byname(const char * name)
{
	return(read_etc_hosts(name, 0));
}




int open_nameservers()
{
	FILE *fp;
	int i;
	#define RESOLV_ARGS 5
	char szBuffer[128], *p, *argv[RESOLV_ARGS];
	int argc;

	if (nameservers > 0)
	    return 0;

	if ((fp = fopen("/etc/resolv.conf", "r")) ||
			(fp = fopen("/etc/config/resolv.conf", "r"))) {

		while (fgets(szBuffer, sizeof(szBuffer), fp) != NULL) {

			for (p = szBuffer; *p && isspace(*p); p++)
				/* skip white space */;
			if (*p == '\0' || *p == '\n' || *p == '#') /* skip comments etc */
				continue;
			argc = 0;
			while (*p && argc < RESOLV_ARGS) {
				argv[argc++] = p;
				while (*p && !isspace(*p) && *p != '\n')
					p++;
				while (*p && (isspace(*p) || *p == '\n')) /* remove spaces */
					*p++ = '\0';
			}

			if (strcmp(argv[0], "nameserver") == 0) {
				for (i = 1; i < argc && nameservers < MAX_SERVERS; i++) {
					nameserver[nameservers++] = strdup(argv[i]);
					DPRINTF("adding nameserver %s\n", argv[i]);
				}
			}

			/* domain and search are mutually exclusive, the last one wins */
			if (strcmp(argv[0],"domain")==0 || strcmp(argv[0],"search")==0) {
				while (searchdomains > 0) {
					free(searchdomain[--searchdomains]);
					searchdomain[searchdomains] = NULL;
				}
				for (i=1; i < argc && searchdomains < MAX_SEARCH; i++) {
					searchdomain[searchdomains++] = strdup(argv[i]);
					DPRINTF("adding search %s\n", argv[i]);
				}
			}
		}
		fclose(fp);
	} else {
	    DPRINTF("failed to open %s\n", "resolv.conf");
	}
	DPRINTF("nameservers = %d\n", nameservers);
	return 0;
}


struct hostent *gethostbyname_t(const char *name)
{
	static struct hostent h;
	static char namebuf[256];
	static struct in_addr in;
	static struct in_addr *addr_list[2];
	struct hostent *hp;
	unsigned char *packet;
	struct resolv_answer a;
	int i;
	int nest = 0;

	open_nameservers();

	if (!name)
		return 0;

	if ((hp = get_hosts_byname(name))) /* do /etc/hosts first */
	{
		findinhosts=1;
		return(hp);
	}

	memset(&h, 0, sizeof(h));

	addr_list[0] = &in;
	addr_list[1] = 0;

	strncpy(namebuf, name, sizeof(namebuf));

	/* First check if this is already an address */
	if (inet_aton(name, &in)) {
	    h.h_name = namebuf;
	    h.h_addrtype = AF_INET;
	    h.h_length = sizeof(in);
	    h.h_addr_list = (char **) addr_list;
	    return &h;
	}

	for (;;) {

		i = dns_lookup(namebuf, 1, nameservers, nameserver, &packet, &a);

		if (i < 0)
			return 0;

		strncpy(namebuf, a.dotted, sizeof(namebuf));
		free(a.dotted);


		if (a.atype == T_CNAME) {		/* CNAME */
			DPRINTF("Got a CNAME in gethostbyname()\n");
			i = decode_dotted(packet, a.rdoffset, namebuf, sizeof(namebuf));
			free(packet);

			if (i < 0)
				return 0;
			if (++nest > MAX_RECURSE)
				return 0;
			continue;
		} else if (a.atype == T_A) {	/* ADDRESS */
			memcpy(&in, a.rdata, sizeof(in));
			h.h_name = namebuf;
			h.h_addrtype = AF_INET;
			h.h_length = sizeof(in);
			h.h_addr_list = (char **) addr_list;
			free(packet);
			break;
		} else {
			free(packet);
			return 0;
		}
	}
	return &h;
}

struct hostent *he;
struct in_addr a;
/*
void save_dns(char *filename , char *domain, char *name)
{
	FILE *fp;
	unsigned char buf[100];	

	fp =fopen(filename, "wa");
	if(fp == NULL)
	{
		fp = fopen(filename, "wb");
	}

	sprintf(buf, "%s ", domain);
	fwrite(buf, strlen(buf), 1, fp);
	sprintf(buf, "%s\r", name);
	fwrite(buf, strlen(buf), 1, fp);
	fclose(fp);
}
*/

//ret =0 fail ret=1 sucess
//ip url=www.XXX dns= namserver_ip tmp_ip= result_ip

int gethost_byname1(const char *url, const char *dns, char *tmp_ip)
{
	FILE *fp;
	unsigned char buf[100];	
	int ret=0;

	fp =fopen("/etc/resolv.conf",  "wb+");
	if(fp==NULL)
	{
		printf("err creat file /etc/resolv.conf\n");
		return 0;
	}
	
	sprintf(buf, "nameserver %s", dns);
	fwrite(buf, strlen(buf), 1, fp);
	fclose(fp);
	system("sync");


	he = gethostbyname_t(url);
    if (he)
    {
        printf("name: %s\n", he->h_name);
        while (*he->h_addr_list)
        {
            bcopy(*he->h_addr_list++, (char *) &a, sizeof(a));
			sprintf(tmp_ip, "%s", inet_ntoa(a));
			printf("ip = %s\n", tmp_ip);
        }
        ret=1;
	}
	return ret;
}

/*
int gethost_byname2(const char *url, const char *dns)
{
	FILE *fp;
	unsigned char buf[100];	
	int ret=0;

	fp =fopen("/etc/resolv.conf",  "wb+");		//rcreat resolv.conf
	if(fp==NULL)
	{
		printf("err creat file /etc/resolv.conf\n");
		return 0;
	}
	
	sprintf(buf, "nameserver\t%s", dns);
	fwrite(buf, strlen(buf), 1, fp);
	fclose(fp);
	system("sync");


	findinhosts=0;
	he = gethostbyname_t(url);
    if (he)
    {
        printf("name: %s\n", he->h_name);
        while (*he->h_addr_list)
        {
            bcopy(*he->h_addr_list++, (char *) &a, sizeof(a));
			//sprintf(tmp_ip, "%s", inet_ntoa(a));
			//printf("ip = %s\n", tmp_ip);
			printf("ip = %s\n", inet_ntoa(a));
		}
        ret=1;
		if(findinhosts==0) //if  url ip not in hosts we record it
		{
			//append url ip in /etc/hosts
			fp =fopen("/etc/hosts",  "a+");
			if(fp==NULL)
			{
				printf("err append url in file /etc/hosts\n");
				return 0;
			}
			sprintf(buf, "%s\t%s\n", inet_ntoa(a) ,url);	//ip url
			fwrite(buf, strlen(buf), 1, fp);
			fclose(fp);
			system("sync");
		}
	}
	else
	{
		printf("can not resolve the url!!!\n");
	}
	return ret;
}
*/

/*
int main (int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s hostname\n", argv[0]);
        return -1;
    }
    strcpy(name_arg, argv[1]);
    he = gethostbyname_t(argv[1]);
    if (he)
    {
        printf("name: %s\n", he->h_name);
        //while (*he->h_aliases)
          //  printf("alias: %s\n", *he->h_aliases++);
        while (*he->h_addr_list)
        {
            bcopy(*he->h_addr_list++, (char *) &a, sizeof(a));
            printf("address: %s\n", inet_ntoa(a));
        }
        save_dns("tmp_dns", argv[1], inet_ntoa(a));		

    }
    else
        herror(argv[0]);
	return 0;        
}
*/

/*

int main(void)
{
	gethostbyname_t("www.sina.com.cn");
}
*/

/*
int main(int argc, char *argv[])
{
	int i;
	if(argc<3)				
	{
		printf("err argc \n");
		return 0;
	}
	for(i=0; i<argc; i++)
		printf("%s\n", argv[i]);
	//return (gethost_byname1(argv[1], argv[2], argv[3]));
	return (gethost_byname2(argv[1], argv[2]));
}*/
