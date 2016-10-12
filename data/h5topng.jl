using HDF5
using Images

filename = "Data.h5"

fid = h5open(filename)

for time in names(fid)
	group = fid[time]
	data = group["Temperature"] |> read
	data -= minimum(data)
	data /= maximum(data)
	save(time*".png", grayim(data))
end

close(fid)
