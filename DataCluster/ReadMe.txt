﻿/***********************************************************************/
/*             动态链接库：DataNode.EXE 项目概述                       */
/***********************************************************************/


DataNode项目综述：
	a) 行情级联服务程序:
			可以通过更换挂载的数据采集插件，来配置级联布局。 
			比如:{交易所数据采集插件+DataNode.exe} --> {内部协议数据采集插件+DataNode.exe} --> {内部协议数据采集插件+DataNode.exe} ...
	b) 主要组成部分：
			本模块是一个框架程序。 即一个封装了数据采集接收模块、行情数据内存管理模块、数据压缩模块的socket服务框架(支持并发连接)。
	c) 模块的监控和控制：
			支持注册到ServiceManager上进行统一管理和监控(数据查询、丢包数据的补发、手动重新初始化等)。


项目设计要求点：
	a) 通用多市场数据格式: 
		可用于所有市场,只需定制对应的“数据采集模块”，将数据以Type+Size+Body形式下发给本框架程序，以实现数据管理的通用性。
	b) 通用内存管理：
		通过内存数据库插件，对所有类型的行情数据格式进行统一管理。用Type+Size描述和分配所需的内存，动态分配查询各行情数据。
	c) 多市场初始化流程管理： 
		只需要通过配置文件，即可灵活的定制初始化“业务流程”。
	d) 支持定制的数据压缩: 
		通过挂载不同的“行情数据压缩模块”，进行多市场的数据压缩。


图示：
/*****************DataNode服务框架************************************/
/*   /****************/   /**********/   /**************/            */
/*   /*初始化管理逻辑*/ + /*定时落盘*/ + /*数据推送逻辑*/            */
/*   /****************/   /**********/   /**************/            */
/*                                                                   */===>级联DataNode.EXE
/* /************/         /******************/      /**************/ */
/* /*数据采集器*/========>/*自适应内存库插件*/=====>/*数据压缩插件*/ */
/* /************/         /******************/      /**************/ */
/*                                                                   */
/*********************************************************************/


////////////////////////////////////////////////////////////////////////////////////////////////////////


数据采集器模块
	概述：
		a) 数据采集模块对外提供初始化接口，方便以被动方式进行初始化。
		b) 调用者提供OnQuery接口方便采集模块可以在只获取增量行情的情况下，进行差量更新。
		c) 数据采集模块通过OnImage接口将初始化行情信息下发给外部调用程序用以初始化，用OnImage的参数bLastFlag来标识初始化是否已经完成
		d) 数据采集模块通过OnData接口将实时行情数据下发给外部调用模块用以更新和行情的转发。

	主要接口说明：
	a) 数据采器器初始化动作激发接口：
	/**
	 * @brief				这是一个数据采集DLL的导出接口，供调用者开始初始化操作的调启
	 */
	extern "C" int			__stdcall RecoverQuotation();

	a) 行情初始化数据回调接口：
	/**
 	 * @brief				初始化性质的行情数据回调
	 * @note				只是更新构造好行情数据的内存初始结构，不推送
							&
							会导致框架模块的重新初始化状态
	 * @param[in]			nDataID				消息ID
	 * @param[in]			pData				数据内容
	 * @param[in]			nDataLen			长度
	 * @param[in]			bLastFlag			是否所有初始化数据已经发完，本条为最后一条的，标识
	 * @return				==0					成功
							!=0					错误
	 */
	virtual int				OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag ) = 0;

	b) 实时行情推送接口:
	/**
	 * @brief				实行行情数据回调
	 * @note				更新行情内存块，并推送
	 * @param[in]			nDataID				消息ID
	 * @param[in]			pData				数据内容
	 * @param[in]			nDataLen			长度
	 * @param[in]			bPushFlag			推送标识
	 * @return				==0					成功
							!=0					错误
	 */
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bPushFlag ) = 0;

	/**
	 * @brief				内存数据查询接口
	 * @param[in]			nDataID				消息ID
	 * @param[in,out]		pData				数据内容(包含查询主键)
	 * @param[in]			nDataLen			长度
	 * @return				>0					成功,返回数据结构的大小
							==0					没查到结果
							!=0					错误
	 */
	virtual int				OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen ) = 0;


初始化流程管理：
	a) 程序入口: 通过调用 DataIOEngine::Activate(), 初始化各资源对象 和 业务控制线程。
			    1) 资源对象：		对下网络服务框架；行情数据采集模块；内存数据插件；
			    2) 业务调试线程：	在Activate()函数完成后，程序的流程控制就完全由该“线程独立调度”。

	b) 业务调度线珵:
		流程：
			1) 先根据“初始化策略”判断是否需要初始化，如果需要初始化，则进入步骤2)，否则进入步骤3)，注：重复初始化间隔默认为3秒且可配置。
			2) 初始化流程：
			   a) 先尝试从本地的历史存盘文件加载所有历史上行情数据到内存插件中。
					注：加载时会校验加载文件的日期，必须是当天的文件或上个交易日期的文件才有效进而被加载。
			   b) 再初始化数据采集模块，从真实行情中更新覆盖真实行情到内存插件。
			   c) 将存盘文件商品代码集合 - 数据采集模块的商品代码集合 = 差集部分的商品代码，从内存插件中删除。
			3) 空闲处理业务:
			   a) 新链接到来时的全行情/增量行情推送。
			   b) 在交易时段内定时进行行情数据定时的落盘操作。

	c) 初始化策略：
	     a) 用配置文件，为各市场配置若干交易时间段(Trading Period)，并注明初始化标识(哪个Trading Period的开始点需从数据源重新初始化行情数据)。
			注：这种配置方法可通用于各个不同市场。
	     b) 加载节假日文件，节假日不需要初始化。(注：需要考虑大节假日，放假天数>=3)
			1) 国内商品期货和期权(大连、郑州、上海、中金），大节假日，前一天无夜盘，不需要初始化。
			2) 国外的暂时未涉及。
		 c) 支持周末双休日测试标识（TestFlag,即有安排测试行情的时候，周末也可进入初始化状态)


定时落盘:
	在初始化流程管理线程的空闲函数中，周期性调用落盘操作。
	注：默认落盘间隔为5分钟且可配置。


数据推送逻辑:
	a) 两个发送缓存: 一个实时行情推送缓存(带实时行情发送独立线程)，一个初始化数据发送缓存。
	b) 两个缓存发送数据的顺序： 在新下级连接到达时，先对下更新全幅或差量行情数据; 再用实时行情发送线程实时推送行情数据。


自适应内存库插件:
	a) 提供 Databases/Tables/Records 为概念对象的数据管理方式。
	b) 使用方可以创建多个Database，并创建n个数据表，并为其插入/删除/更新/查询m条数据记录。 
	c) 模块保证table中的数据记录是连续的内存块。以方便对下级发送整块的连续的内存行情数据。
	d) 该模块线程安全。


数据压缩插件:
	a) 将行情数据压缩模块插件化，方便升级。


////////////////////////////////////////////////////////////////////////////////////////////////////////







