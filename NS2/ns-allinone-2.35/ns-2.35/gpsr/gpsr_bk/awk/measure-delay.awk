#measure-delay.awk
#這是測量CBR封包端點到端點間延遲時間的awk程式
BEGIN{
#程式初始化，設定一變數以記錄目前最高處理封包的ID
 highest_packet_id = 0;
} 
{
 action = $1;            # +/- 表示進入/離開了佇列   r/d表示封包被某個節點接收/丟棄 
 time = $2;               # 事件發生的時間
 from = $3;              # 事件發生地點 (from node)
 to = $4;                   # 事件發生地點 (to node)
 type = $5;               # 封包的型態
 pktsize = $6;         # 封包的大小
 flow_id = $8;         # 封包屬於哪個資料流
 src = $9;                # 封包的來源端   (a.b) a = 節點編號, b = port number
 dst = $10;              # 封包的目的端   (a.b) a = 節點編號, b = port number
 seq_no = $11;      # 封包的序號
 packet_id = $12;  # 封包的ID
#紀錄目前最高的packet ID
if( packet_id > highest_packet_id )
 highest_packet_id = packet_id;
#紀錄封包的傳送時間
if( start_time[packet_id] == 0 )
 start_time[packet_id] = time;
#紀錄CBR (flow_id = 2) 的接收時間
if(flow_id == 2 && action != "d"){
 if(action == "r"){
  end_time[packet_id] = time;
 }
}else{
#把不是flow_id = 2的封包或者是flow_id = 2但此封包被drop的，時間設為-1
 end_time[packet_id] = -1;
}

}
END{
#當資料列全部讀完後，開始計算有效封包的端點到端點延遲時間
for(packet_id = 0; packet_id <= highest_packet_id; packet_id++){
 start = start_time[packet_id];
 end   = end_time[packet_id];
 packet_duration = end - start;
#只把接收時間大於傳送時間的紀錄列出來
if(start < end)
printf("%f %f\n", start, packet_duration)
}
}

# 取自於柯志亨老師-計算機網路實驗   一書
