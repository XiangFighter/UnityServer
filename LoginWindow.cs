using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class LoginWindow : MonoBehaviour {
    public InputField inputUserName;
    public InputField inputPassword;
    // Use this for initialization
    void Start()
    {
        //告诉网络命令管理类当前对象可接收网络命令
        NetworkCommand.GetInstance().NetworkCommandResponsors.Add(gameObject);
    }
    private void OnDestroy()
    {
        //告诉网络命令管理类当前对象可接收网络命令
        NetworkCommand.GetInstance().NetworkCommandResponsors.Remove(gameObject);
    }
    //// Update is called once per frame
    //void Update () {

    //}
    public void OnLogin()
    {
        string userName = inputUserName.text;
        if (userName == null || userName.Length < 3)
        {
            //提示用户名错误
            //MessageBox("用户名错误");
            print("用户名不合法");
            return;
        }
        string password = inputPassword.text;
        if (password == null || password.Length < 3)
        {
            //提示用户名错误
            //MessageBox("密码错误");
            print("密码不合法");
            return;
        }
        NetworkCommand.GetInstance().
            ExcudeCommand("Login", new string[] { userName, password });
    }
    public void OnCancel()
    {
        Application.Quit();
    }
   public void LoginResult(string [] result)
    {
        print("result:" + result[0]);
        if (result[0].Contains("Success"))
        {
            //存储本地用户名
            PlayerPrefs.SetString("UserName", inputUserName.text);
            SceneManager.LoadScene("mainScene");
        }
        else
        {
            //提示用户名错误
            //MessageBox("用户名或密码错误");
            print("用户名或密码错误");
            return;
        }
   }
}
