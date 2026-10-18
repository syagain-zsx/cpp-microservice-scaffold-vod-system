create database if not exists `vptest`;
use `vptest`;

INSERT INTO `tbl_identify_meta` VALUES 
    (null, 'identity_id_0', 0, '未知身份'), 
    (null, 'identity_id_1', 1, 'C端用户'), 
    (null, 'identity_id_2', 2, 'B端用户');

INSERT INTO `tbl_role_meta` VALUES 
    (null, 'role_id_0', 0, '未知角色'),
    (null, 'role_id_1', 1, '超管角色'),
    (null, 'role_id_2', 2, '普管角色'),
    (null, 'role_id_3', 3, '普通角色'),
    (null, 'role_id_4', 4, '访客角色');

insert tbl_user_meta values
    (null, 'user_id_1', '000@qq.com', 'zhanglaoliu', null, null, '123123', 1, 'avatar_fid_1', null, now()),
    (null, 'user_id_2', '111@qq.com', 'lisi', null, null, '123123', 1, 'avatar_fid_2', null, now()),
    (null, 'user_id_3', '222@qq.com', 'wangwu', null, null, '123123', 1, 'avatar_fid_3', null, now()),
    (null, 'user_id_4', '333@qq.com', 'zhaoliu', null, null, '123123', 1, 'avatar_fid_4', null, now()),
    (null, 'user_id_5', '444@qq.com', 'sunba', null, null, '123123', 1, 'avatar_fid_5', null, now()),
    (null, 'user_id_6', '555@qq.com', 'qianqi', null, null, '123123', 1, 'avatar_fid_6', null, now()),
    (null, 'user_id_7', '666@qq.com', 'tianjiu', null, null, '123123', 1, 'avatar_fid_7', null, now()),
    (null, 'user_id_8', '777@qq.com', 'liushi', null, null, '123123', 1, 'avatar_fid_8', null, now()),
    (null, 'user_id_9', '888@qq.com', 'zhangshiyi', null, null, '123123', 1, 'avatar_fid_9', null, now());

insert tbl_user_identify_role_meta values
    (null, 'user_id_1', 1, 3),
    (null, 'user_id_2', 2, 1),
    (null, 'user_id_2', 1, 3),
    (null, 'user_id_3', 2, 2),
    (null, 'user_id_3', 1, 3),
    (null, 'user_id_4', 2, 2),
    (null, 'user_id_4', 1, 3),
    (null, 'user_id_5', 2, 2),
    (null, 'user_id_5', 1, 3),
    (null, 'user_id_6', 2, 2),
    (null, 'user_id_6', 1, 3),
    (null, 'user_id_7', 2, 2),
    (null, 'user_id_7', 1, 3),
    (null, 'user_id_8', 2, 2),
    (null, 'user_id_8', 1, 3),
    (null, 'user_id_9', 2, 2),
    (null, 'user_id_9', 1, 3);
