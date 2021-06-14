package demo_server

type MluResult struct {
	Image string `json:"image"`
	Time  string `json:"time"`
	Boxes string `json:"boxes"`
}

func (mluResult *MluResult) getImage() string {
	return mluResult.Image
}

func (mluResult *MluResult) getTime() string {
	return mluResult.Time
}

func (mluResult *MluResult) getBoxes() string {
	return mluResult.Boxes
}
