str2 = input ("", "s");
str = "";

while !strcmp (str2, "}")
	str = strcat (str, str2);
	str2 = input ("", "s");
end
str = strcat (str, str2);

J = jsondecode (str)

sa_begin = str2double (J.LSS_STEERING_ANGLE_BEGIN)
sa_end = str2double (J.LSS_STEERING_ANGLE_END)
lr_bias_end = str2double (J.LSS_LR_BIAS_END)

steering_angle = linspace(-110, 110, 100);
lr_bias = ((max (min (abs (steering_angle), sa_end), sa_begin)) - sa_begin) * ((lr_bias_end - 0.5) / (sa_end - sa_begin)) + 0.5;

waitfor (plot (steering_angle, lr_bias));