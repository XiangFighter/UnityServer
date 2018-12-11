using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using UnityEngine;

public class NetworkCommand : MonoBehaviour {
    public List<GameObject> NetworkCommandResponsors;//接收命令的对象
    byte[] data = new byte[8096];
    public string serverIP = "127.0.0.1";
    public int serverPort = 8080;
    Socket clientSocket=null;
    public static NetworkCommand instance=null;
    public static NetworkCommand GetInstance() { return instance; }
    bool readFinished;
    string readString;
    // Use this for initialization
    void Start () {
        if (instance != null&&instance!=this)
        {
            Destroy(gameObject);
        }
        instance = this;
        NetworkCommandResponsors = new List<GameObject>();
        DontDestroyOnLoad(gameObject);
        if (instance.clientSocket == null)
        {
            instance.clientSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        }
        clientSocket.BeginReceive(data, 0, 8096, SocketFlags.None, ReceiveCallBack, null);
        StartCoroutine(WaitForResponseRoutine());
    }
    public void ExcudeCommand(string commandName, object[] parameters)
    {
        if (!instance.clientSocket.Connected)
        {
            clientSocket.Connect(new IPEndPoint(IPAddress.Parse(serverIP), serverPort));
        }
        string messageString = commandName + "(";
        foreach (var para in parameters)
        {
            messageString += para.ToString() + ",";
        }
      //  messageString += ")";
        byte[] buffer = Encoding.UTF8.GetBytes(messageString);
        ////发送消息
        clientSocket.Send(buffer);
    }
    //由于网络回调函数不会在Unity主线程中，无法与Unity3d界面元素交互，不能在此函数内调用业务回调函数
    private void ReceiveCallBack(IAsyncResult result)
    {
        int bytesRead = clientSocket.EndReceive(result);
        readString = System.Text.Encoding.UTF8.GetString(data, 0, bytesRead);
        readFinished = true;
    }
    IEnumerator WaitForResponseRoutine()
    {
        while (gameObject.activeSelf)
        {
            yield return new WaitUntil(delegate ()
            {
                return readFinished;
            });
            //处理命令
            var substr = readString.Split('(');
            var parameters = substr[1].Split(',');
            foreach (var responsor in NetworkCommandResponsors)
            {
                responsor.SendMessage(substr[0], parameters, SendMessageOptions.DontRequireReceiver);
            }
            //重置状态
            readFinished = false;
            //重新发送接收请求
            clientSocket.BeginReceive(data, 0, 8096, SocketFlags.None, ReceiveCallBack, null);
        }
    }
}
