
/**
 * @brief 
 * 
 * @param buff 
 * @param len 
 * @return int 
 */
int mc_sock_send(int mc_send_v4, unsigned char *buff, int len)
{
    int ret = 0;
    int rport = 6600;
    struct sockaddr_in cli_addr;
    char *multicast_ipv4_addr = "232.10.20.1";

    cli_addr.sin_family = AF_INET;
    inet_pton(AF_INET, multicast_ipv4_addr, &cli_addr.sin_addr.s_addr);
    cli_addr.sin_port = htons(rport);

    ret = sendto(mc_send_v4, buff, len, 0,
                (struct sockaddr*)&cli_addr, sizeof(cli_addr));
    log_debug("ret: %d", ret);
    if (-1 == ret)
    {
//         ps->mc_send_err_cnt++;
    } else {
//         ps->mc_send_bytes += ret;
//         ps->mc_send_cnt ++;
//         ret = 0;
    }

    return ret;
}

/**
 * @brief 
 * 
 * @param mc_sock 
 * @param ifname 
 * @param sport 
 * @param rport 
 * @param mc_ipv4 
 * @return int 
 */
int mc_sock_sender_init(int *mc_sock, char *ifname,
        int sport, int rport, char *mc_ipv4)
{
    int ret = 0;
    struct sockaddr_in ser_addr,cli_addr;
    struct ip_mreqn group;  // 像这个组播组发送数据
    int fd = -1;
    int loop = 0;

    fd = socket(AF_INET, SOCK_DGRAM, 0);  // 构建UDP通信套接字
    if (-1 == fd)
    {
        log_warn("socket error: %s", strerror(errno));
        return -1;
    }

    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(sport);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // 绑定地址结构
    ret = bind(fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));  // 绑定服务器地址结构
    if (-1 == ret)
    {
        log_warn("bind() error: %s", strerror(errno));
        close(fd);
        return -2;
    }

    inet_pton(AF_INET, mc_ipv4, &group.imr_multiaddr);  // 设置组地址
    inet_pton(AF_INET, "0.0.0.0", &group.imr_address);    // 本地任意IP 相当于INADDR_ANY
    group.imr_ifindex = if_nametoindex(ifname); // 给出网卡名，转为对应的编号
    ret = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &group, sizeof(group)); // 套接字组播权限
    if (-1 == ret)
    {
        log_warn("setsockopt() error?: %s", strerror(errno));
        close(fd);
        return -3;
    }

    /**
     * @brief 
     * WINDOWS 中 该选项仅控制接收部分。即设置为0 则控制套接字无法接收自身消息。设置为1 则控制套接字使能接收自身消息。
     * LINUX   中 该先项仅控制发送部分。即设置为0 则控制套接字无法向自身发送消息。设置为1 则控制套接字可以向自身发送消息。
     * 
     * WINDOWS如果该选项，设置在接收套接字上
     * LINUX 设置在发送套按字上
     */
    ret = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    if (-1 == ret)
    {
        log_warn("setsockopt() for IP_MUTICAST_LOOP error: %s", strerror(errno));
        close(fd);
        return -3;
    }

    // 给客户端绑定固定地址组播地址
    cli_addr.sin_family = AF_INET;
    inet_pton(AF_INET, mc_ipv4, &cli_addr.sin_addr.s_addr);
    cli_addr.sin_port = htons(rport);

#if 0
    int i = 0;
    char buf[1024]= "hello";
    while(1)
    {
        // 写给对端
        sprintf(buf, "hello %d\n", i++);
        sendto(fd, buf, strlen(buf), 0, (struct sockaddr*)&cli_addr, sizeof(cli_addr));
        sleep(1);
    }
    close(fd);
#endif
    *mc_sock = fd;
    
    return ret;
}


/**
 * @brief 
 * 
 * @param mc_sock 
 * @param ifname 
 * @param sport 
 * @param rport 
 * @param mc_ipv4 
 * @return int 
 */
int mc_sock_receiver_init(int *mc_sock, char *ifname,
        int sport, int rport, char *mc_ipv4)
{
    int ret = 0;
    struct ip_mreqn group;
    struct sockaddr_in localaddr;
    int fd = -1;
    int loop = 0;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == fd)
    {
        log_warn("socket() error: %s", strerror(errno));
        return -1;
    }

    log_debug("rport: %d", rport);
    localaddr.sin_family = AF_INET;
    localaddr.sin_port = htons(rport);
    inet_pton(AF_INET, "0.0.0.0", &localaddr.sin_addr.s_addr);

    ret = bind(fd, (struct sockaddr*)&localaddr, sizeof(localaddr));
    if (-1 == ret)
    {
        log_warn("bind() error: %s", strerror(errno));
        close(fd);
        return -2;
    }

    log_debug("mc_ipv4: %s", mc_ipv4);

    inet_pton(AF_INET, mc_ipv4, &group.imr_multiaddr);  // 设置组地址
    inet_pton(AF_INET, "0.0.0.0", &group.imr_address);    // 当前客户端ip加入组播组
    group.imr_ifindex = if_nametoindex(ifname); // 给出网卡名，转为对应的编号

    ret = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group)); // 将客户端加入组播组
    if (-1 == ret)
    {
        log_warn("setsockopt() error: %s", strerror(errno));
        close(fd);
        return -3;
    }
    // ret = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    // if (-1 == ret)
    // {
    //     log_warn("setsockopt() for IP_MUTICAST_LOOP error: %s", strerror(errno));
    //     close(fd);
    //     return -3;
    // }

    *mc_sock = fd;

#if 0
    char buf[1024];
    int len=0;
    while (1)
    {
        log_debug("");
        len = recvfrom(fd, buf, sizeof(buf), 0, NULL, 0);
        log_debug("len: %d", len);
        write(STDOUT_FILENO, buf, len);
    }

    close(fd);
    return 0;
#endif
    return ret;
}
