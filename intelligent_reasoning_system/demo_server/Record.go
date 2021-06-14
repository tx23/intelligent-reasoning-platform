package demo_server

type Record struct {
	Ip         string `json:"ip" gorm:"column:Ip"`
	Type       string `json:"type" gorm:"column:Type"`
	Ptime      string `json:"ptime" gorm:"column:Ptime"`
	Ctime      string `json:"ctime" gorm:"column:Ctime"`
	Fps        string `json:"fps" gorm:"column:Fps"`
	PowerWaste string `json:"powerwaste" gorm:"column:PowerWaste"`
	MemUsage   string `json:"memusage" gorm:"column:MemUsage"`
	Id         string `json:"id" gorm:"primary_key;column:Id"`
	Batch      string `json:"batch" gorm:"column:Batch"`
}

func (record *Record) getIp() string {
	return record.Ip
}

func (record *Record) getType() string {
	return record.Type
}

func (record *Record) getPtime() string {
	return record.Ptime
}

func (record *Record) getCtime() string {
	return record.Ctime
}

func (record *Record) getMemUsage() string {
	return record.MemUsage
}

func (record *Record) getId() string {
	return record.Id
}

func (record *Record) getBatch() string {
	return record.Batch
}
