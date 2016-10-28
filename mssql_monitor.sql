
select cntr_value from sys.sysperfinfo 
where counter_name in ('Full Scans/sec', 'Average Latch Wait Time (ms)', 'User Connections', 'Processes blocked') or 
(counter_name in ('buffer cache hit ratio', 'buffer cache hit ratio base','Lazy Writes/sec', 'Page reads/sec',
 'Page writes/sec', 'Database pages') and object_name like '%buffer manager%') or 
(counter_name in ('Cache Hit Ratio', 'Cache Hit Ratio base') and instance_name = 'test') or 
(counter_name in ('Number of Deadlocks/sec', 'Average Wait Time (ms)', 'Lock Requests/sec') and instance_name = '_Total') 
order by object_name, counter_name