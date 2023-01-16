using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Data.SqlClient;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using MySql.Data.MySqlClient;

namespace RC_Car_DB
{
    public partial class Form1 : Form
    {
        string Conn = "Server=localhost;port=3310;Database=rc_car_db;Uid=root;Pwd=1234;Allow User Variables=True;";
        string start, end, user = "홍길동";
        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (textBox1.Text == "")
            {
                MessageBox.Show("입력하세요!");
            }
            else
            {
                if (serialPort1.IsOpen == false)
                {
                    serialPort1.PortName = textBox1.Text;
                    serialPort1.Open();
                    // DB 삭제
                    using (MySqlConnection conn = new MySqlConnection(Conn))
                    {
                        conn.Open();
                        MySqlCommand msc = new MySqlCommand("delete from drive_data", conn);
                        msc.ExecuteNonQuery();

                        MySqlCommand msc2 = new MySqlCommand("alter table drive_data auto_increment=0;", conn);
                        msc2.ExecuteNonQuery();
                    }
                    // 시작시간, username
                    start = DateTime.Now.ToString();
                }
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen == true)
            {
                // 종료시간
                end = DateTime.Now.ToString();

                // 총 대여시간
                DateTime StartDate = Convert.ToDateTime(start);
                DateTime EndDate = Convert.ToDateTime(end);

                TimeSpan dateDiff = EndDate - StartDate;

                int diffDay = dateDiff.Days;
                int diffHour = dateDiff.Hours;
                int diffMinute = dateDiff.Minutes;
                int diffSecond = dateDiff.Seconds;

                int rental_time = (int)(diffDay * 86400 + diffHour * 3600 + diffMinute * 60 + diffSecond);

                //DB에 데이터 삽입
                using (MySqlConnection conn = new MySqlConnection(Conn))
                {
                    conn.Open();
                    MySqlCommand msc = new MySqlCommand("insert into car_data(user, start, end, rental_time) values('" + user + "','" + start + "','" + end + "', " + rental_time + ")", conn);
                    msc.ExecuteNonQuery();
                }

                serialPort1.Close();   
            }
        }

        private void serialPort1_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            string rawdata = serialPort1.ReadLine();
            string[] data = rawdata.Split('/');


            if (label2.InvokeRequired)
            {
                label2.Invoke(new MethodInvoker(delegate { label2.Text = "sensor : " + data[0]; }));
            }
            else
                label2.Text = "sensor : " + data[0];

            if (label3.InvokeRequired)
            {
                label3.Invoke(new MethodInvoker(delegate { label3.Text = "distance : " + data[1]; }));
            }
            else
                label3.Text = "distance : " + data[1];

            if (label4.InvokeRequired)
            {
                label4.Invoke(new MethodInvoker(delegate { label4.Text = "deltha : " + data[2]; }));
            }
            else
                label4.Text = "deltha : " + data[2];


            string date = DateTime.Now.ToString();

            //DB에 데이터 삽입
            using (MySqlConnection conn = new MySqlConnection(Conn))
            {
                conn.Open();
                MySqlCommand msc = new MySqlCommand("insert into drive_data(date,collision,distance,deltha) values('" + date + "'," + data[0] + "," + data[1] + "," + data[2] + ")", conn);
                msc.ExecuteNonQuery();
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            using (MySqlConnection conn = new MySqlConnection(Conn))
            {
                DataSet ds = new DataSet();
                string sql = "select * from drive_data";
                MySqlDataAdapter adpt = new MySqlDataAdapter(sql, conn);
                adpt.Fill(ds, "drive_data");

                listView1.Items.Clear();
                chart1.Series[0].Points.Clear();
                chart1.Series[1].Points.Clear();
                chart1.Series[2].Points.Clear();

                for (int i = 0; i < ds.Tables[0].Rows.Count; i++)
                {
                    //한 레코드씩 리스트뷰에 집어넣음
                    ListViewItem lvi = new ListViewItem();
                    lvi.Text = ds.Tables[0].Rows[i]["num"].ToString();

                    string date = ds.Tables[0].Rows[i]["date"].ToString();
                    string collision = ds.Tables[0].Rows[i]["collision"].ToString();
                    string distance = ds.Tables[0].Rows[i]["distance"].ToString();
                    string deltha = ds.Tables[0].Rows[i]["deltha"].ToString();

                    lvi.SubItems.Add(date);
                    lvi.SubItems.Add(collision);
                    lvi.SubItems.Add(distance);
                    lvi.SubItems.Add(deltha);

                    listView1.Items.Add(lvi);

                    //한 레코드씩 그래프에 출력
                    chart1.Series[0].Points.AddXY(i,double.Parse(collision));
                    chart1.Series[1].Points.AddXY(i, double.Parse(distance));
                    chart1.Series[2].Points.AddXY(i, double.Parse(deltha));
                }
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            if(listView1.Items.Count > 0)
            {
                string path = "./output.csv";
                FileStream fs = new FileStream(path,FileMode.Create);
                StreamWriter sw = new StreamWriter(fs);

                for(int i=0;i<listView1.Items.Count;i++)
                {
                    string line = "";
                    for(int j=0;j<4;j++)
                    {
                        line += listView1.Items[i].SubItems[j].Text+",";
                    }
                    line += listView1.Items[i].SubItems[4].Text;

                    sw.WriteLine(line);
                }

                sw.Close();
                sw.Dispose();
                fs.Close();

                fs.Dispose();

            }
        }

        private void textBox1_TextChanged(object sender, EventArgs e) {}
        private void listView1_SelectedIndexChanged(object sender, EventArgs e) {}
        private void label2_Click(object sender, EventArgs e) {}
        private void label3_Click(object sender, EventArgs e) {}
        private void label2_Click_1(object sender, EventArgs e) {}
        private void label4_Click(object sender, EventArgs e){}
        private void label5_Click(object sender, EventArgs e){}
    }
}
