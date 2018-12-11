using System.Collections;
using System.Collections.Generic;
using UnityEngine;
[RequireComponent(typeof(CharacterController))]
public class ControllPlayer : MonoBehaviour {
    public static List<ControllPlayer> existUsers;
    public GameObject userPrefab;//玩家预制体
    public float speedX = 0.01f, speedZ = 0.01f;
    CharacterController controller;
    Vector3 speed=Vector3.zero;
    bool keyStateUp, keyStateDown, keyStateLeft, keyStateRight;
    public string userName;
    public bool isLocalUser = false;
    // Use this for initialization
    void Start () {
        if (existUsers == null) existUsers = new List<ControllPlayer>();
        existUsers.Add(this);
        controller = GetComponent<CharacterController>();
        //告诉网络命令管理类当前对象可接收网络命令
        NetworkCommand.GetInstance().NetworkCommandResponsors.Add(gameObject);

        if (isLocalUser)
        {
            keyStateUp = Input.GetKey(KeyCode.UpArrow);
            keyStateDown = Input.GetKey(KeyCode.DownArrow);
            keyStateLeft = Input.GetKey(KeyCode.LeftArrow);
            keyStateRight = Input.GetKey(KeyCode.RightArrow);
            userName = PlayerPrefs.GetString("UserName");
            print(userName + " is LocalUser\n");
            //请求用户列表
            NetworkCommand.GetInstance().ExcudeCommand("RequestUserList", new string[] { userName });
        }
    }
    private void OnDestroy()
    {
        //告诉网络命令管理类当前对象可接收网络命令
        NetworkCommand.GetInstance().NetworkCommandResponsors.Remove(gameObject);
    }
    private void FixedUpdate()
    {
        //本地玩家响应键盘输入
        if (isLocalUser)
        {
            //只有在改变输入状态时才更新速度，防止网络拥塞
            if (keyStateUp != Input.GetKey(KeyCode.UpArrow) ||
            keyStateDown != Input.GetKey(KeyCode.DownArrow) ||
            keyStateLeft != Input.GetKey(KeyCode.LeftArrow) ||
            keyStateRight != Input.GetKey(KeyCode.RightArrow))
            {
                keyStateUp = Input.GetKey(KeyCode.UpArrow);
                keyStateDown = Input.GetKey(KeyCode.DownArrow);
                keyStateLeft = Input.GetKey(KeyCode.LeftArrow);
                keyStateRight = Input.GetKey(KeyCode.RightArrow);
                Vector3 s = Vector3.zero;
                if (keyStateDown) s.z -= speedZ;
                if (keyStateUp) s.z += speedZ;
                if (keyStateLeft) s.x -= speedX;
                if (keyStateRight) s.x += speedX;
                ChangeSpeed(s);
            }
        }
        // transform.position += speed;
        controller.Move(speed);
    }
    /// <summary>
    /// 接受服务器通知修改玩家速度
    /// </summary>
    /// <param name="user">玩家</param>
    /// <param name="s">速度</param>
    public void NotifyChangeSpeed(string [] parameters)
    {
        string user = parameters[0];
        Vector3 s = new Vector3(float.Parse(parameters[1]), float.Parse(parameters[2]), float.Parse(parameters[3]));
        Vector3 p= new Vector3(float.Parse(parameters[4]), float.Parse(parameters[5]), float.Parse(parameters[6]));
        var userInfo= existUsers.Find(u => u.userName == user);
        if (userInfo!=null)
        {
            userInfo.transform.position = p;//同步位置，由于网络延迟等原因可能产生位置不一致
            userInfo.speed = s;//更新速度
            
        }
     }
    public void ChangeSpeed(Vector3 s)
    {
        //修改本地速度
        speed = s;
        //通知服务器修改本人速度
        NetworkCommand.GetInstance().ExcudeCommand("NotifyChangeSpeed", new string[] { userName,
            s.x.ToString(), s.y.ToString(), s.z.ToString(),
            transform.position.x.ToString(),transform.position.y.ToString(),transform.position.z.ToString()
        });//不使用s.ToString()防止产生小括号到参数
    }
    /// <summary>
    /// 当其它用户登录时添加用户实例
    /// </summary>
    /// <param name="parameters"></param>
    public void AddUser(string [] parameters)
    {
        if (isLocalUser)
        {
            if (existUsers.Find(player => player.userName == parameters[0]) != null) return;
            GameObject newUser = Instantiate(userPrefab);
            newUser.transform.position = new Vector3(float.Parse(parameters[1]), float.Parse(parameters[2]), float.Parse(parameters[3]));
            ControllPlayer control = newUser.GetComponent<ControllPlayer>();
            control.isLocalUser = false;
            control.userName = parameters[0];
            control.speed= new Vector3(float.Parse(parameters[4]), float.Parse(parameters[5]), float.Parse(parameters[6]));
           
        }
    }
}
