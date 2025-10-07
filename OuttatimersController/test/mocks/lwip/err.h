#pragma once
// Mock LWIP error definitions for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

// LWIP error types
typedef int err_t;

#define ERR_OK          0    /* No error, everything OK. */
#define ERR_MEM        -1    /* Out of memory error.     */
#define ERR_BUF        -2    /* Buffer error.            */
#define ERR_TIMEOUT    -3    /* Timeout.                 */
#define ERR_RTE        -4    /* Routing problem.         */
#define ERR_INPROGRESS -5    /* Operation in progress    */
#define ERR_VAL        -6    /* Illegal value.           */
#define ERR_WOULDBLOCK -7    /* Operation would block.   */
#define ERR_USE        -8    /* Address in use.          */
#define ERR_ALREADY    -9    /* Already connecting.      */
#define ERR_ISCONN     -10   /* Conn already established.*/
#define ERR_CONN       -11   /* Not connected.           */
#define ERR_IF         -12   /* Low-level netif error    */
#define ERR_ABRT       -13   /* Connection aborted.      */
#define ERR_RST        -14   /* Connection reset.        */
#define ERR_CLSD       -15   /* Connection closed.       */
#define ERR_ARG        -16   /* Illegal argument.        */

#ifdef __cplusplus
}
#endif