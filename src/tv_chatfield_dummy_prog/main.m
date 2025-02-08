WIDTH = 21;
HEIGHT = 31;

printf ("{\n\t\"CHATFIELD_LUT\":\n\t[\n");

for yi = 0:(HEIGHT-1)

	printf ("\t\t[");

    for xi = 0:(WIDTH-1)

        x = xi * 5;
        y = yi * 1 - 15;
        z = (y / 30) / (1 + exp (0.1 * (50 - x))) + 0.5;

        printf ("\"%.2f\"", z);
        if (xi != WIDTH - 1)
            printf (", ");
		end;

    end;

	printf ("]");
    if (yi != HEIGHT - 1)
        printf (",");
	end;

	printf ("\n");

end;

printf ("\t]\n}\n");
