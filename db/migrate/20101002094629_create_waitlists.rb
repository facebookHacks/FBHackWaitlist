class CreateWaitlists < ActiveRecord::Migration
  def self.up
    create_table :waitlists do |t|
      t.string :name
      t.string :phone
      t.string :email
      t.string :crn

      t.timestamps
    end
  end

  def self.down
    drop_table :waitlists
  end
end
