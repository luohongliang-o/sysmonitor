select * from sys.sysperfinfo
where 1 = 1
and counter_name in (
--General Statistics
'User Connections',
'Processes blocked',
'Temp Tables For Destruction'
)
union
select * from sys.sysperfinfo
where 1 = 1
and object_name like '%Buffer Manager%' 
and counter_name in (
'Buffer cache hit ratio',
'Database pages',
'Page life expectancy',
'Lazy writes/sec'
)
union 
select * from sys.sysperfinfo
where 1 =1
and (instance_name = '_Total')
and counter_name in (
--Locks
'Lock Requests/sec',
'Lock Timeouts/sec',
'Number of Deadlocks/sec'
)
union
select * from sys.sysperfinfo
where 1 =1
and (instance_name = 'test')
and counter_name in (
--Database
'Transactions/sec'
)
order by counter_name 