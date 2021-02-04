#ifndef AWS_CLIENT_CREDENTIAL_KEYS_H
#define AWS_CLIENT_CREDENTIAL_KEYS_H

#include <stdint.h>

/*
 * PEM-encoded client certificate.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----"
 */
// static char keyCLIENT_CERTIFICATE_PEM[] = "\0";
//"-----BEGIN CERTIFICATE-----\n"
//"MIIDWjCCAkKgAwIBAgIVAONLT4gTvFiiwOQP12ZiA8QBsRh2MA0GCSqGSIb3DQEB\n"
//"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n"
//"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMDA2MDIxMzI5\n"
//"NDJaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n"
//"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDasglN+a2hFI4Z3FIZ\n"
//"uAaVsaVEX7t6p+1mX5drXkvssXJHHpeg1Ao2IP9xcl3RLozdqCDzbwP85S07TBdB\n"
//"gRfxI52SgATm6Nq/8Cu/abx/Ee05qbTKt6cKr8mmVnw4Ti7U4RIBafgcLcl7Be+S\n"
//"w4XuIHFi8q3czVmaRoIXDSy1ka4yqPxJ2eK+zkVT01P0APbEH1dVl/VqBtaen6So\n"
//"i5uu9M3UhmBdpim4SfXA9spNh3MVnI+VJzfHbyRhV2Oc2gwTSMSYR1L1U0iOhsD9\n"
//"AHJZ66s4G0cBy3aeuQtsQW+A7ZGLN3As7ljyDp7mKar37Gz375J0it7+zWCdbW5h\n"
//"o9xxAgMBAAGjYDBeMB8GA1UdIwQYMBaAFMc9UuDrTosLbOefan7gzhWxVLkwMB0G\n"
//"A1UdDgQWBBQpUP7p5SwpdfKjG7Vokr2VU928yjAMBgNVHRMBAf8EAjAAMA4GA1Ud\n"
//"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAJ5uLofYcezGN+MRzfnC6U+L1\n"
//"neEO2DKpxYAHLrKnG3yU5OJsPK4tpjLFNCVO3mFZNrAF7ZulvRefPKD6QDMWtykw\n"
//"zd+KMh58fFSrZDdA/Owdmu6ItJX3UJQp/N5+vWD+8PF52GmsW51XCI14TCKTHFEN\n"
//"0bWJa89efYxEUj72jI5PFTao8ZlD5qmz467sgB0c413l6IAf5xG4BbjY1YOtVYvE\n"
//"08GGSRQfKB4n3dM6D45zgEkgIM5S0OoQ6qJyaVjQhAICAYHG42TUCNDvD2e9KXWF\n"
//"tC2tgfEnNClJduvgj7RYocQJzwRNZlJdUorcEaHP31NcLIJ+rWIQGtZAU/1GPg==\n"
//"-----END CERTIFICATE-----\n";

/*
 * PEM-encoded client private key.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN RSA PRIVATE KEY-----\n"\
 * "...base64 data...\n"\
 * "-----END RSA PRIVATE KEY-----"
 */
// static char keyCLIENT_PRIVATE_KEY_PEM[] = "\0";
//"-----BEGIN RSA PRIVATE KEY-----\n"
//"MIIEpQIBAAKCAQEA2rIJTfmtoRSOGdxSGbgGlbGlRF+7eqftZl+Xa15L7LFyRx6X\n"
//"oNQKNiD/cXJd0S6M3agg828D/OUtO0wXQYEX8SOdkoAE5ujav/Arv2m8fxHtOam0\n"
//"yrenCq/JplZ8OE4u1OESAWn4HC3JewXvksOF7iBxYvKt3M1ZmkaCFw0stZGuMqj8\n"
//"Sdnivs5FU9NT9AD2xB9XVZf1agbWnp+kqIubrvTN1IZgXaYpuEn1wPbKTYdzFZyP\n"
//"lSc3x28kYVdjnNoME0jEmEdS9VNIjobA/QByWeurOBtHAct2nrkLbEFvgO2Rizdw\n"
//"LO5Y8g6e5imq9+xs9++SdIre/s1gnW1uYaPccQIDAQABAoIBAQDJfJYN/Sb27VUu\n"
//"hkot39pROGYnZHv6OZUDaLa8+RfCbon29DyGtFTkIeq2vsOo4dZusWQKZBGsggj2\n"
//"RNh4RVOlm0alnsTlaUuA4umrZOOBvyZspeoniqSft+11DuFLjtyezO4l11f7vkOO\n"
//"0J00/mb6SXGt1CBS+e6/sI5SZpT2ghkcPGTMF/NZPh4przE7yGFpf2BnhfLx4VZx\n"
//"ks4pepV2JvW2dHGfX161xbW9QQwoEYl+OGbwzY1cFlIUpuyB2kpsmLRkMC2zuhNH\n"
//"W83wcBY6AWCxsW/MrA5FyTZrW4yACVLHZIaWKI4O4BDNzaKg3ay9vWUBhjNXQyCz\n"
//"wmIVWNMBAoGBAPgRl5QXMFH81DBulKT/ZIdiMhWk9lZOkCpA39P4niJZVkTa2jd5\n"
//"YB1PC7afDCGEo1kJHeZwGqndWqrh+lf3ce++qxLvXJSgu5GxBT/o59cjWNREg8C5\n"
//"nv9AdJhE6wFY+OEWbiAtC1gmHSV7gzBRiHTPI/otItU6NnevTWEX70HJAoGBAOGw\n"
//"BzXK/LxsOYMDRiIUTRPhHi0OInwE8ML6PnfRyseh2W9lDKP/7hJtQr4tvgzspqKB\n"
//"B9+HKACRp+ASS1Fyd1bCiFrkebgOUH1Zr3mqkSiK8pSGq6YbAwW3aE0ChxZGY9QM\n"
//"9vmzEJ9ilVYDw94fU2BcCYBe91cYj691wJSXC1lpAoGBAIE08IWC2nkpvf+H6UZT\n"
//"e7IEVF/vrxrfCrnnVm3axcrQS13Lu/M+9e/uFwTErJXctxQN6pP2+fxVvf7ZewJM\n"
//"cPzVi9dt8wO7AFdvI3PZRMBKud693P4J8KW0lcYlx13JUMA0ZaG+tHTyTSYTwg3t\n"
//"uaxVo8CPt9/l1S7bkOssg0L5AoGBANBb7HwsWGM442viJOinp3V3+50LFKk9Am5w\n"
//"DCYg6L6f3RdrylzKlqgqeVkM8A4MmdcjmvoyVKsZzkwfd6vNKwJNfQe1Me7MQGFJ\n"
//"4KhJ4dP1W/nlzyj+fS7U4hVfBQp6mAWYppdnSyxrHoYX2cgDUb1/m0IJ6v5c7aLG\n"
//"6rIENPUpAoGAGruKHjRygHKVrWAux7cKeOpC8evqP518sMn4oBFyMqkcNQzenuC4\n"
//"Ol7o9kzFTrvlwyFVpfrdt9MT2irQ/5F+Rdg1Xfc4m2bqRR99DH5PaxkWV5PiYtcW\n"
//"RXBpNyYNonEv9dyEE/4huVEJwRZq8ul0hYDzx3NwanWYKxYkVjXV500=\n"
//"-----END RSA PRIVATE KEY-----\n";

/*
 * PEM-encoded Just-in-Time Registration (JITR) certificate (optional).
 *
 * If used, must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----\n"\
 * "...base64 data...\n"\
 * "-----END CERTIFICATE-----"
 */
//#define keyJITR_DEVICE_CERTIFICATE_AUTHORITY_PEM  ""


#endif /* AWS_CLIENT_CREDENTIAL_KEYS_H */
