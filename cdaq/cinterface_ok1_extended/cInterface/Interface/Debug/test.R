data<-read.table("DAQdata001.txt",skip=1)
names(data)<-c("fx","fy","fz","tx","ty","tz","emg1","emg2","emg3","emg4","pulse")
freq<-10000
data$time<-seq(-500/freq,length=nrow(data),by=1/freq)
data<-data[data$time<0.2,]

plot(data$time,data$fy,type="l",xlim=c(-.01,0.2),ylim=c(-8,-1))
abline(v=0)
lines(data$time,data$pulse/5-2,col=2)

plot(data$time,data$fy,type="l",xlim=c(-.01,0.02),ylim=c(-4,-1))
abline(v=0)


ems<-read.table("EMSdata001.txt",skip=1)
names(ems)<-c("emstime","pos","pulse","pctime")

ems<-ems[ems$emstime!=0,]

plot(ems$emstime,ems$pctime)

fit<-lm(pctime~emstime,ems)
ems$time<-ems$emstime*coef(fit)[2]

time0<-ems$time[which(diff(ems$pulse)==1)]
ems$time<-ems$time-time0

ems$pos<-ifelse(ems$pos>180,ems$pos-360,ems$pos)

par(mfrow=c(2,1),mar=c(1,5,1,1),oma=c(4,1,1,1))

plot(ems$time,ems$pos,type="l",xlim=c(-.01,0.2),xlab="",xaxt="n")
abline(v=0)
lines(ems$time,ems$pulse,col=2)

plot(data$time,data$fy,type="l",xlim=c(-.01,0.2),ylim=c(-8,-1))
abline(v=0)
lines(data$time,data$pulse/5-2,col=2)


