/**
 * @file udp_interface.h
 * @brief UDP interface definitions to send and receive data over the
 * network via UDP.
 */
#ifndef UDP_INTERFACE_H_
#define UDP_INTERFACE_H_

/**
 * @brief 1NCE onboarding endpoint.
 * @brief 1NCE onboarding port.
 */

/**
 * @brief general network endpoint
 *
 */
struct OSEndPoint
{
    /**
     * @brief The remote EndPoint hostname.
     *
     */
    const char host[ 100 ];

    /**
     * @brief The remote EndPoint port number
     *
     */
    int port;
};
typedef struct OSEndPoint   OSEndPoint_t;

/**
 *
 * @typedef OSNetwork_t
 * @brief The OSNetwork is an incomplete type. An implementation of this
 * interface must define struct OSNetwork for the system requirements.
 * This OSNetwork is passed into the network interface functions.
 */
struct OSNetwork;
typedef struct OSNetwork    * OSNetwork_t;


/**
 *  @brief os_network_ops: The operations to be implemented.
 */
struct os_network_ops
{
    /**
     * @brief Implementation-defined network socket.
     */
    OSNetwork_t os_socket;

    /**
     * @brief UDP interface for connecting to 1NCE server.
     *
     *
     * @param[in] osnetwork Implementation-defined network socket.
     * @param[in] nce_oboarding General network endpoint.
     *
     * @return @return The status of the connection.
     */
    int (* nce_os_udp_connect)( OSNetwork_t osnetwork,
                                OSEndPoint_t nce_oboarding );

    /**
     * @brief Sends data over an established UDP connection.
     *
     *
     * @param[in] OSNetwork_t Implementation-defined network socket.
     * @param[in] pBuffer Buffer containing the bytes to send.
     * @param[in] bytesToSend Number of bytes to send from the buffer.
     *
     * @return Number of bytes (> 0) sent on success;
     * 0 if the socket times out without sending any bytes;
     * else a negative value to represent error.
     */
    int (* nce_os_udp_send)( OSNetwork_t osnetwork,
                             void * pBuffer,
                             size_t bytesToSend );

    /**
     * @brief Receives data from an established UDP connection.
     *
     * @param[in] osnetwork Implementation-defined network socket.
     * @param[out] pBuffer Buffer to receive bytes into.
     * @param[in] bytesToRecv Number of bytes to receive from the network.
     *
     * @return Number of bytes (> 0) received if successful;
     * 0 if the socket times out without reading any bytes;
     * negative value on error.
     */
    int (* nce_os_udp_recv)( OSNetwork_t osnetwork,
                             void * pBuffer,
                             size_t bytesToRecv );

    /**
     * @brief Gracefully disconnect an established UDP connection.
     *
     * @param[in] osnetwork Implementation-defined network socket.
     */
    int (* nce_os_udp_disconnect)( OSNetwork_t osnetwork );
};

typedef struct os_network_ops os_network_ops_t;



#endif /* ifndef UDP_INTERFACE_H_ */
