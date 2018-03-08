/* See LICENSE file for copyright and license details. */
#include <stdint.h>

#if defined(HAVE_ENDIAN_H)
# include <endian.h>
#elif defined(HAVE_SYS_ENDIAN_H)
# include <sys/endian.h>
#endif

#if !defined(HAVE_ENDIAN_H) && !defined(HAVE_SYS_ENDIAN_H)

# if defined(HOST_ENDIAN_IS_LITTLE_ENDIAN_16)
#  if !defined(htole16)
#   define htole16(x) (x)
#  endif
#  if !defined(htole16)
#   define letoh16(x) (x)
#  endif
# endif
# if defined(HOST_ENDIAN_IS_LITTLE_ENDIAN_32)
#  if !defined(htole32)
#   define htole32(x) (x)
#  endif
#  if !defined(htole32)
#   define letoh32(x) (x)
#  endif
# endif
# if defined(HOST_ENDIAN_IS_LITTLE_ENDIAN_64)
#  if !defined(htole64)
#   define htole16(x) (x)
#  endif
#  if !defined(htole64)
#   define letoh16(x) (x)
#  endif
# endif

# if !defined(htole16)
#  define htole16 blind_htole16
static inline uint16_t
blind_htole16(uint16_t h)
{
	union {
		unsigned char bytes[2];
		uint16_t value;
	} d;
	d.bytes[0] = h;
	d.bytes[1] = h >> 8;
	return d.value;
}
# endif

# if !defined(htole32)
#  define htole32 blind_htole32
static inline uint32_t
blind_htole32(uint32_t h)
{
	union {
		unsigned char bytes[4];
		uint32_t value;
	} d;
	d.bytes[0] = h;
	d.bytes[1] = h >> 8;
	d.bytes[2] = h >> 16;
	d.bytes[3] = h >> 24;
	return d.value;
}
# endif

# if !defined(htole64)
#  define htole64 blind_htole64
static inline uint64_t
blind_htole64(uint64_t h)
{
	union {
		unsigned char bytes[8];
		uint64_t value;
	} d;
	d.bytes[0] = h;
	d.bytes[1] = h >> 8;
	d.bytes[2] = h >> 16;
	d.bytes[3] = h >> 24;
	d.bytes[4] = h >> 32;
	d.bytes[5] = h >> 40;
	d.bytes[6] = h >> 48;
	d.bytes[7] = h >> 56;
	return d.value;
}
# endif

# if !defined(le16toh)
#  if defined(letoh16)
#   define le16toh letoh16
#  else
#   define le16toh blind_le16toh
static inline uint16_t
blind_le16toh(uint16_t le)
{
	unsigned char *bytes = (unsigned char *)&le;
	return ((uint16_t)(bytes[1]) << 8) | (uint16_t)(bytes[0]);
}
#  endif
# endif

# if !defined(le32toh)
#  if defined(letoh32)
#   define le32toh letoh32
#  else
#   define le32toh blind_le32toh
static inline uint32_t
blind_le32toh(uint32_t le)
{
	unsigned char *bytes = (unsigned char *)&le;
	return ((uint32_t)(bytes[3]) << 24) |
	       ((uint32_t)(bytes[2]) << 16) |
	       ((uint32_t)(bytes[1]) << 8) |
		(uint32_t)(bytes[0]);
}
#  endif
# endif

# if !defined(le64toh)
#  if defined(letoh64)
#   define le64toh letoh64
#  else
#   define le64toh blind_le64toh
static inline uint64_t
blind_le64toh(uint64_t le)
{
	unsigned char *bytes = (unsigned char *)&le;
	return ((uint64_t)(bytes[7]) << 56) |
	       ((uint64_t)(bytes[6]) << 48) |
	       ((uint64_t)(bytes[5]) << 40) |
	       ((uint64_t)(bytes[4]) << 32) |
	       ((uint64_t)(bytes[3]) << 24) |
	       ((uint64_t)(bytes[2]) << 16) |
	       ((uint64_t)(bytes[1]) << 8) |
	       (uint64_t)(bytes[0]);
}
#  endif
# endif

#elif defined(HAVE_OPENBSD_ENDIAN)
# define le16toh letoh16
# define le32toh letoh32
# define le64toh letoh64
#endif
