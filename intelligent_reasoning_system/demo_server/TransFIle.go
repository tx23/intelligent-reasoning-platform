package demo_server

type TransFile struct {
	//文件Id
	FileId uint64
	//网络序号
	NetId uint64
	// 文件头
	DataLen uint64
	// 文件内容
	Data []byte
}

func NewTransFile(fileId, netId uint64, data []byte) *TransFile {
	return &TransFile{
		FileId: fileId,
		NetId: netId,
		DataLen: uint64(len(data)),
		Data: data,
	}
}

func (transFile *TransFile) getFileId() uint64 {
	return transFile.FileId
}

func (transFile *TransFile) getNetId() uint64 {
	return transFile.NetId
}

func (transFile *TransFile) getDataLen() uint64 {
	return transFile.DataLen
}

func (transFile *TransFile) getData() []byte {
	return transFile.Data
}