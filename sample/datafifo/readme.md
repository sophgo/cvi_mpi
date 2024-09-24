sample datafifo 使用说明：
sample datafifo包含大核sample_reader与小核sample_writer两部分，使用步骤：
1.在cvi_alios相应solution的package_yaml中配置CONFIG_APP_TEST为1
2.在cvi_alios/components/cvi_test/package_yaml中配置CONFIG_APP_TEST为1
3.编译得到烧录镜像及sample_reader可执行程序
4.烧录并进入系统后挂载sd卡
5.在alios小核端中执行datafifo_write_start，产生如下类似输出：
        (cli-uart)# datafifo_write_start
        PhyAddr: 82900280
        datafifo_init finish
        send: ========0========
6.获取PhyAddr并在Linux大核端执行./sample_reader 82900280
7.测试完毕后小核端执行datafifo_write_stop，大核端输入q退出程序