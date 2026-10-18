use `vptest`;

insert tbl_video_meta values
    (null, 'video_1', 'cover_fid_1', 'video_fid_1', 'user_id_1', 'title_1', null, null, 12, 60, 1024, 2, now()),
    (null, 'video_2', 'cover_fid_2', 'video_fid_2', 'user_id_1', 'title_2', null, null, 18, 60, 1024, 2, now()),
    (null, 'video_3', 'cover_fid_3', 'video_fid_3', 'user_id_1', 'title_3', null, null, 30, 60, 1024, 2, now()),
    (null, 'video_4', 'cover_fid_4', 'video_fid_4', 'user_id_2', 'title_4', null, null, 88, 60, 1024, 2, now()),
    (null, 'video_5', 'cover_fid_5', 'video_fid_5', 'user_id_2', 'title_5', null, null, 22, 60, 1024, 2, now()),
    (null, 'video_6', 'cover_fid_6', 'video_fid_6', 'user_id_3', 'title_6', null, null, 88, 60, 1024, 2, now());
insert tbl_video_meta values(null, 'video_7', 'cover_fid_7', 'video_fid_7', 'user_id_4', 'title_7', null, null, 88, 60, 1024, 2, now());
insert tbl_video_meta values(null, 'video_8', 'cover_fid_8', 'video_fid_8', 'user_id_4', 'title_8', null, null, 88, 60, 1024, 2, now());
insert tbl_video_meta values(null, 'video_9', 'cover_fid_9', 'video_fid_9', 'user_id_5', 'title_9', null, null, 88, 60, 1024, 2, now());
insert tbl_video_meta values(null, 'video_10', 'cover_fid_10', 'video_fid_10', 'user_id_5', 'title_10', null, null, 88, 60, 1024, 2, now());
insert tbl_video_meta values(null, 'video_11', 'cover_fid_11', 'video_fid_11', 'user_id_5', 'title_11', null, null, 88, 60, 1024, 2, now());
insert tbl_video_like_meta values
    (null, 'user_id_1', 'video_1'),
    (null, 'user_id_2', 'video_1'),
    (null, 'user_id_3', 'video_1'),
    (null, 'user_id_4', 'video_1'),
    (null, 'user_id_1', 'video_2'),
    (null, 'user_id_2', 'video_2'),
    (null, 'user_id_1', 'video_3'),
    (null, 'user_id_3', 'video_3'),
    (null, 'user_id_1', 'video_4'),
    (null, 'user_id_2', 'video_4'),
    (null, 'user_id_3', 'video_4'),
    (null, 'user_id_2', 'video_5'),
    (null, 'user_id_3', 'video_6');


insert  tbl_video_catgory_tag_meta values
    (null, 'video_1', 1, 1),
    (null, 'video_2', 1, 1),
    (null, 'video_3', 1, 1),
    (null, 'video_1', 1, 2),
    (null, 'video_2', 1, 2),
    (null, 'video_3', 1, 2),
    (null, 'video_1', 1, 3),
    (null, 'video_3', 1, 3),
    (null, 'video_4', 1, 3),
    (null, 'video_5', 1, 3),
    (null, 'video_6', 2, 4),
    (null, 'video_7', 2, 4);