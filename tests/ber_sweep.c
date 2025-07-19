#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sched.h>

#include "noise.h"
#include "turbofec/turbo.h"

#define MAX_LEN_BITS		32768
#define MAX_LEN_BYTES		(32768/8)
#define DEFAULT_NUM_PKTS	1000
#define DEFAULT_ITER		4

/* Maximum LTE code block size of 6144 */
#define LEN		TURBO_MAX_K

#define DEFAULT_AMP	32.0

const struct lte_turbo_code lte_turbo = {
	.n = 2,
	.k = 4,
	.len = LEN,
	.rgen = 013,
	.gen = 015,
};

struct lte_test_vector {
	const char *name;
	const char *spec;
	const struct lte_turbo_code *code;
	int in_len;
	int out_len;
};

const struct lte_test_vector tests[] = {
	{
		.name = "3GPP LTE turbo",
		.spec = "(N=2, K=4)",
		.code = &lte_turbo,
		.in_len  = LEN,
		.out_len = LEN * 3 + 4 * 3,
	},
	{ /* end */ },
};

static void fill_random(uint8_t *b, int n)
{
	int i, r, m, c;

	c = 0;
	r = rand();
	m = sizeof(int) - 1;

	for (i = 0; i < n; i++) {
		if (c++ == m) {
			r = rand();
			c = 0;
		}

		b[i] = (r >> (i % m)) & 0x01;
	}
}

static int uint8_to_err(int8_t *dst, uint8_t *src, int n, float snr)
{
	int i, err = 0;

	add_noise(src, dst, n, snr, DEFAULT_AMP);

	for (i = 0; i < n; i++) {
		if ((!src[i] && (dst[i] >= 0)) || (src[i] && (dst[i] <= 0)))
			err++;
	}

	return err;
}

static int error_test(const struct lte_test_vector *test,
		      int num_pkts, int iter, float snr,
		      float *ber, float *fer)
{
	int i, n, l, iber = 0, ober = 0, frame_err = 0;
	int8_t *bs0, *bs1, *bs2;
	uint8_t *in, *bu0, *bu1, *bu2;

	in  = malloc(sizeof(uint8_t) * MAX_LEN_BITS);
	bu0 = malloc(sizeof(uint8_t) * MAX_LEN_BITS);
	bu1 = malloc(sizeof(uint8_t) * MAX_LEN_BITS);
	bu2 = malloc(sizeof(uint8_t) * MAX_LEN_BITS);
	bs0 = malloc(sizeof(int8_t) * MAX_LEN_BITS);
	bs1 = malloc(sizeof(int8_t) * MAX_LEN_BITS);
	bs2 = malloc(sizeof(int8_t) * MAX_LEN_BITS);

	struct tdecoder *tdec = alloc_tdec();

	for (i = 0; i < num_pkts; i++) {
		fill_random(in, test->in_len);
		l = lte_turbo_encode(test->code, in, bu0, bu1, bu2);
		if (l != test->out_len) {
			fprintf(stderr, "[!] Failed encoding length check (%i)\n",
				l);
			return -1;
		}

		iber += uint8_to_err(bs0, bu0, LEN + 4, snr);
		iber += uint8_to_err(bs1, bu1, LEN + 4, snr);
		iber += uint8_to_err(bs2, bu2, LEN + 4, snr);

		lte_turbo_decode_unpack(tdec, LEN, iter, bu0, bs0, bs1, bs2);

	        for (n = 0; n < test->in_len; n++) {
			if (in[n] != bu0[n])
				ober++;
		}

		if (memcmp(in, bu0, test->in_len))
			frame_err++;
	}

	*ber = (float) ober / (num_pkts * test->in_len);
	*fer = (float) frame_err / num_pkts;

	free(in);
	free(bs0);
	free(bs1);
	free(bs2);
	free(bu0);
	free(bu1);
	free(bu2);

	return 0;
}

struct cmd_options {
	int num_pkts;
	int iter;
	float snr_min;
	float snr_max;
	float snr_step;
};

static void print_help()
{
	fprintf(stdout, "Options:\n"
		"  -h    This text\n"
		"  -p    Number of packets (default: %i)\n"
		"  -i    Number of turbo iterations (default: %i)\n"
		"  -s    SNR start value in dB (default: 0.0)\n"
		"  -e    SNR end value in dB (default: 8.0)\n"
		"  -t    SNR step value in dB (default: 0.5)\n",
		DEFAULT_NUM_PKTS, DEFAULT_ITER);
}

static void handle_options(int argc, char **argv, struct cmd_options *cmd)
{
	int option;

	cmd->num_pkts = DEFAULT_NUM_PKTS;
	cmd->iter = DEFAULT_ITER;
	cmd->snr_min = 0.0;
	cmd->snr_max = 8.0;
	cmd->snr_step = 0.5;

	while ((option = getopt(argc, argv, "hp:i:s:e:t:")) != -1) {
		switch (option) {
		case 'h':
			print_help();
			exit(0);
			break;
		case 'p':
			cmd->num_pkts = atoi(optarg);
			break;
		case 'i':
			cmd->iter = atoi(optarg);
			break;
		case 's':
			cmd->snr_min = atof(optarg);
			break;
		case 'e':
			cmd->snr_max = atof(optarg);
			break;
		case 't':
			cmd->snr_step = atof(optarg);
			break;
		default:
			print_help();
			exit(0);
		}
	}
}

int main(int argc, char *argv[])
{
	const struct lte_test_vector *test = &tests[0];
	struct cmd_options cmd;
	float snr, ber, fer;

	handle_options(argc, argv, &cmd);
	srandom(time(NULL));

	printf("# SNR (dB), BER, FER\n");

	for (snr = cmd.snr_min; snr <= cmd.snr_max; snr += cmd.snr_step) {
		error_test(test, cmd.num_pkts, cmd.iter, snr, &ber, &fer);
		printf("%f, %f, %f\n", snr, ber, fer);
	}

	return 0;
}