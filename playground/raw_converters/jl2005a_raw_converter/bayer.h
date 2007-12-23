typedef enum {
	BAYER_TILE_RGGB,
	BAYER_TILE_GRBG,
	BAYER_TILE_BGGR,
	BAYER_TILE_GBRG,
	BAYER_TILE_RGGB_INTERLACED,
	BAYER_TILE_GRBG_INTERLACED,
	BAYER_TILE_BGGR_INTERLACED,
	BAYER_TILE_GBRG_INTERLACED
} BayerTile;

int
gp_bayer_decode (unsigned char *input, int w, int h, unsigned char *output,
		 BayerTile tile);
