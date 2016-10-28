show status where variable_name in
(
-- 吞吐量
'Questions'                           -- 已执行语句（由客户端发出）计数  
,'Com_select'                         -- SELECT 语句
,'Com_insert'                          
,'Com_update'
,'Com_delete'                         -- Writes 插入，更新或删除 
,'Uptime'                             -- mysql 服务器运行时间
,'Slow_queries'
-- 连接
,'Threads_connected'                  -- 当前开放的连接
,'Threads_running'                    -- 当前运行的连接
,'Aborted_connects'                   -- 尝试与服务器进行连接结果失败的次数
-- 缓冲池
,'Innodb_buffer_pool_pages_total'     -- 缓冲池中的总页数
,'Innodb_buffer_pool_pages_free'      -- 缓冲池中剩余页数
,'Innodb_buffer_pool_read_requests'   -- 向缓冲池发送的请求
,'Innodb_buffer_pool_reads'           -- 缓冲池无法满足的请求
-- 网络流量
,'Bytes_received'                     -- 接收到的字节数
,'Bytes_sent'                         -- 发送的字节数
,'Connections'                        -- 试图连接到(不管是否成功)MySQL服务器的连接数
,'Qcache_free_memory'                 -- 用于查询缓存的自由内存的数量。
,'Qcache_hits'                        -- 查询缓存被访问的次数
,'Queries'                            -- 被服务器执行的语句个数，包括存储过程里的语句，也包括show	status之类的
,'Table_locks_waited'                 -- 不能立即获得的表的锁的次数。如果该值较高，并且有性能问题，你应首先优化查询，然后拆分表或使用复制
)