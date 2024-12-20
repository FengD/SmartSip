# 项目介绍

`SmartSip`是一款智能饮水机。之所以要做一款这样的饮水机主要是因为经过市场调研，市面上的饮水机无法满足我的一些综合性的需求，相关需求可能存在一些共性。
所以我们希望结合大模型，开发这样一款饮水机能够给我的日常用水带来便捷和一些比较有意思的体验。

* 市面上的饮水机从出出水方式上整体分为了以下几种：
  * `最简单的吸水式饮水机`，一般会在水桶顶部安装一个装置，通过按压或者电动抽水的方式在较大的水桶中抽水到水杯或者水壶里。这种类型的装置在用水上比较方便，针对比较大的水桶避免了搬运和卫生问题。缺点是无法进行对水温的控制和加热；
    ![image](https://github.com/user-attachments/assets/46138e05-240b-4e86-ab44-a53fb1624676)
  * 考虑到如果对水有加热的需求，基于上面第一种饮水机会衍生出两类产品。`一类产品是仍然保持对抽水装置的设计，将水抽到一个可用于加热的水壶中，再进行加热`；`再有一类就是可以将水导入一个饮水机自带的制定容器在出水时进行即热调节`。这两种产品都能解决对水使用且加热的需求，但是前者基本上无法实现对水温的精准控制，基本是加热到100°过程中需要手动停止，并且实际使用上想喝到较高温度的水，相对来说也会慢一些，优点是一般会有一个温度传感器，能够看到水温。而第二类虽然可以可以实现速热，但是手动上水的操作确实是个体力活，而且目前市面上能够看到的最大的容器大概1.6L，如果按照一杯水150-300ml计算得话，基本上一个人生活平均一天要上一次水。
    ![image](https://github.com/user-attachments/assets/c9405489-a804-4754-9201-f3fb75ff649f)
  * 目前市面上看得到唯二的饮水器，能够兼顾1、2中的所有功能，但是不能实现对水温的精确控制
    ![image](https://github.com/user-attachments/assets/bb0a96ac-6638-4458-ba6d-045326a2cb6a)

* 除了按照出水方式和使用便捷程度，`价格`和`智能化`也是toC产品的重要卖点。目前市面上还没有跟大模型结合的饮水机品类，同时以上饮水机的价格动辄300-500RMB同样也是一笔不小的开销，所以基于我们对饮水机结构的分析，我们希望自己做一个满足我们自己智能化需求，同时成本不超过200RMB的饮水机，并将整个解决方案开源供大家参考。![image](https://github.com/user-attachments/assets/1797603b-181e-470f-b524-011d7062e97b)

# 产品功能
1. 满足卫生和美观的情况下，直接从所购买的大水桶中进行取水，减少人工对于水桶的搬运，可用于办公室和家庭等场景下对水量和水温的需求；
2. 针对于目前即热功能较粗的温度调节范围、挡位选择和水量调节，通过摄像头+麦克风+多模态大模型的组合实现对于接水的一键控制。整个组合可以做到对于下方容器的检测（包括类型，摆放位置以及是否存在遮挡覆盖物等），容器中物品的检测（粉状物如奶粉、速溶咖啡、抹茶粉，茶叶如红茶、绿茶、白茶、黑茶、果茶、花茶等），从而智能决定水温、水量，加水过程中对加水量进行自动检测，防止溢出。【v0.1完成】
3. 同时因为有语音的加持，还可以通过声音对饮水机进行控制，并且通过对声纹进行识别，进行个性化定制的需求；【待完成】
4. 考虑到智能化识别本身需要一定算力，这个算力运行可以在手机上运行，也可以跟云上的服务进行交互，如果有手机加持，可以考虑添加使用手机上的相机或者额外添加一个相机对使用人进行检测，防止儿童老人因为使用不当或者水温过热，出现安全问题；【v0.1完成】
5. 同时摄像头也能同声纹一样对使用人的饮水习惯进行自定义需求。【待完成】

`亮点：整个产品只有一个触屏作为摄像头可视化和唯一的按键。点击屏幕即可流出满足用户需求的水温和水量的出水`。同时可依赖于本身硬件的无线通讯能力，实现配件拓展，例如自动化茶台，实现多人饮茶的泡茶流程的自动化。

# 交互流程

考虑到端到端检测的需求，我们对大模型进行了应用场景的评测，评测报告见 README-LLM-test-report.md。并针对有可能出现的问题，使用一些边缘端模型尝试进行补偿，如水溢出容器等。
![image](https://github.com/user-attachments/assets/603523df-5c2b-43b2-8057-739b313fa5cd)

# 文档

相关软硬件说明和工程文档以及测试报告模型等均可以在该工程中找到。
