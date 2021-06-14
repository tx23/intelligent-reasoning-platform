package demo_face

// 传输文件接口
type ITransFile interface {
	//获得文件Id
	getFileId() uint32
	//获得网络Id
	getNetId() uint32
	//获得数据长度
	getDataLen() uint32
	//获得数据内容
	getData() []byte
}