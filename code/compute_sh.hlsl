cbuffer switch_buff
{
    uint nb_cell_w;
    uint nb_cell_h;
    uint grps_per_dim;
	uint my_switch;
};

RWStructuredBuffer<uint> first_buff : register(u0);
RWStructuredBuffer<uint> second_buff : register(u1);
RWStructuredBuffer<uint> limits : register(u2);

[numthreads(32, 1, 1)]
void compute_main(uint3 disp_th_id : SV_DispatchThreadID)
{
    uint flat_index = disp_th_id.x + disp_th_id.y * grps_per_dim * 32 + disp_th_id.z * grps_per_dim * grps_per_dim * 32;

    uint  i = flat_index % nb_cell_w;
    uint j = flat_index / nb_cell_w;
    if(i > 0 && j > 0 && i < nb_cell_w -1 && j < nb_cell_h - 1)
    {
        if (!my_switch)
        {       
            int neigh_alive = 0;
            neigh_alive += int(first_buff[(j - 1) * nb_cell_w + i - 1]);
            neigh_alive += int(first_buff[j * nb_cell_w + i - 1]);
            neigh_alive += int(first_buff[(j + 1) * nb_cell_w + i - 1]);
            neigh_alive += int(first_buff[(j - 1) * nb_cell_w + i]);
            neigh_alive += int(first_buff[(j + 1) * nb_cell_w + i]);
            neigh_alive += int(first_buff[(j - 1) * nb_cell_w + i + 1]);
            neigh_alive += int(first_buff[j * nb_cell_w + i + 1]);
            neigh_alive += int(first_buff[(j + 1) * nb_cell_w + i + 1]);

            if (first_buff[j * nb_cell_w + i])
            {
                if (neigh_alive < 2 || neigh_alive > 3)
                    second_buff[j * nb_cell_w + i] = false;
                else
                    second_buff[j * nb_cell_w + i] = true;
            }
            else
            {
                if (neigh_alive == 3)
                {
                    second_buff[j * nb_cell_w + i] = true;
                    if (i < limits[0])
                        limits[0] = i;
                    if (i > limits[2])
                        limits[2] = i;
                    if (j < limits[1])
                        limits[1] = j;
                    if (j > limits[3])
                        limits[3] = j;
                }
                else
                    second_buff[j * nb_cell_w + i] = false;
            }
        }
        else
        {
            int neigh_alive = 0;
            neigh_alive += int(second_buff[(j - 1) * nb_cell_w + i - 1]);
            neigh_alive += int(second_buff[j * nb_cell_w + i - 1]);
            neigh_alive += int(second_buff[(j + 1) * nb_cell_w + i - 1]);
            neigh_alive += int(second_buff[(j - 1) * nb_cell_w + i]);
            neigh_alive += int(second_buff[(j + 1) * nb_cell_w + i]);
            neigh_alive += int(second_buff[(j - 1) * nb_cell_w + i + 1]);
            neigh_alive += int(second_buff[j * nb_cell_w + i + 1]);
            neigh_alive += int(second_buff[(j + 1) * nb_cell_w + i + 1]);

            if (second_buff[j * nb_cell_w + i])
            {
                if (neigh_alive < 2 || neigh_alive > 3)
                    first_buff[j * nb_cell_w + i] = false;
                else
                    first_buff[j * nb_cell_w + i] = true;
            }
            else
            {
                if (neigh_alive == 3)
                {
                    first_buff[j * nb_cell_w + i] = true;
                    if (i < limits[0])
                        limits[0] = i;
                    if (i > limits[2])
                        limits[2] = i;
                    if (j < limits[1])
                        limits[1] = j;
                    if (j > limits[3])
                        limits[3] = j;
                }
                else
                    first_buff[j * nb_cell_w + i] = false;
            }
        }
    }
}